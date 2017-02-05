import threading,os,sys,serial
import pickle
from struct import unpack
#sklearn stuff
from sklearn import svm,preprocessing
#pylab stuff
from pylab import fft,ceil
#audio stuff
from scipy.io import wavfile
import pyaudio
import wave
#time stuff
from time import strftime, sleep, time
from datetime import datetime

from DeviceComm import CommRequest

__location__ = os.path.realpath(os.path.join(os.getcwd(), os.path.dirname(__file__))) #directory from which this script is ran

class AcousticAnalysis():
	calib_location = os.path.join(__location__,'calibration')

	def __init__(self,ardfunc,clf_name,scaler_name):
		self.doneRecording = True
		self.p = pyaudio.PyAudio()
		print self.p.get_default_input_device_info()
		print pyaudio.__file__
		self.recordLock = threading.Lock()
		self.ardfunc = ardfunc
		self.clf_name = clf_name
		self.scaler_name = scaler_name
		self.clf = None
		self.scaler = None
		if not os.path.exists(os.path.join(self.calib_location,'wavfiles')):
			os.mkdir(os.path.join(self.calib_location,'wavfiles'))
		with open(os.path.join(self.calib_location,self.clf_name),'rb') as calibfile:
			self.clf = pickle.load(calibfile)
		with open(os.path.join(self.calib_location,self.scaler_name),'rb') as scalerfile:
			self.scaler = pickle.load(scalerfile)

	#main function for getting back tap result
	def getIfFoam(self):
		isFoam = None
		# start recording
		self.doneRecording = False
		recordReturn = [None,None]
		recordThread = threading.Thread(target=self.recordDuration,args=(0.300,5,os.path.join(self.calib_location,'wavfiles'),None,recordReturn))
		recordThread.daemon = True
		recordThread.start()
		# tell arduino to tap
		resp = self.ardfunc.performTap()
		#wait for recording to finish
		while not self.doneRecording:
			sleep(0.2)
		hasRead,name = recordReturn
		if not hasRead:
			print 'didnt read anything'
			return None
		# after recording, get sample by perform Fourier on file
		sample_list = []
		self.addFourierSample(name,'calibration/wavfiles',sample_list,[],'lol')
		# test sample
		result = self.predictWithFit(sample_list)
		if result[0] == '0':
			isFoam = False
		elif result[0] == '1':
			isFoam = True
		#return result
		return isFoam

	#returns prediction
	def predictWithFit(self,sample):
		sample_scaled = self.scaler.transform(sample)
		return self.clf.predict(sample_scaled)

	def addFourierSample(self,itemwav,filedir,X_list,y_list,y_act):
		fourierData = self.performFourierDataReturn(itemwav,filedir)
		sample = fourierData[3]
		sample = self.getDataWithinFreq(sample,50,2000,fourierData[0],fourierData[2])
		X_list.append(sample)
		y_list.append(y_act)

	def getDataWithinFreq(self,data,minHz,maxHz,sampFreq,sampCount):
		divisor = (sampFreq/float(sampCount))
		minIndex = int(round(minHz/divisor))
		maxIndex = int(round(maxHz/divisor))
		return data[minIndex:maxIndex+1]


	def performFourierDataReturn(self,wav_name,input_location=None):
		if input_location == None:
			input_location = ''
		if wav_name.endswith('.wav'):
			wav_file_name = os.path.join(os.path.join(__location__,input_location),wav_name)
		else:
			wav_file_name = os.path.join(os.path.join(__location__,input_location),wav_name)+'.wav'
		sampFreq, snd = wavfile.read(wav_file_name)
		#convert 16 bit values to be between -1 to 1
		snd = snd / (2.0**15)
		s1 = self.getAverageData(snd)

		n = len(s1)
		p = fft(s1)

		nUniquePts = int(ceil((n+1)/2.0))
		p = p[0:nUniquePts]
		p = abs(p)

		p = p/float(n)
		p = p**2

		if n % 2 > 0:
			p[1:len(p)] = p[1:len(p)] * 2
		else:
			p[1:len(p)-1] = p[1:len(p)-1] * 2

		return (sampFreq,nUniquePts,n,p)

	def getAverageData(self,data):
		newdata = []
		for part1,part2 in zip(data.T[0],data.T[1]):
			newdata.append((part1+part2)/2.0)
		return newdata


	#records sound, returns if recorded a sound
	def recordDuration(self,duration,recordLimit,output_location=None,output_name=None,returnList=None):
		#set done recording to False
		#self.recordLock.acquire()
		self.doneRecording = False
		#self.recordLock.release()
		THRESHOLD = 32000
		CHANNELS = 1
		FORMAT = pyaudio.paInt16
		RATE = 44100
		CHUNK_SIZE = int(RATE*duration)

		now = datetime.now()
		timestamp = now.strftime("%Y%m%d%H%M%S.%f")

		stream = self.p.open(format=FORMAT,
			channels=CHANNELS,
			rate=RATE,
			input=True,
			input_device_index = 9,
			frames_per_buffer = CHUNK_SIZE*3
			)

		#look in block for threshold value
		readFrames = 0
		readLimit = RATE*recordLimit #stop trying to detect after 5 seconds
		hasRead = False
		print "STREAM SAMP WIDTH: %s" % self.p.get_sample_size(FORMAT)
		while readFrames < readLimit:
			found,framesToSave = self.get_next_tap_LIVE(stream,CHUNK_SIZE,CHANNELS,THRESHOLD,RATE,duration)
			if found:
				#create output dir if does not exist
				print output_location
				if not os.path.exists(output_location): os.mkdir(output_location)
				#save with appropriate name
				if output_name != None and output_name.endswith('.wav'):
					wav_save_name = os.path.join(os.path.join(output_location),output_name)
				elif output_name != None and not output_name.endswith('.wav'):
					wav_save_name = os.path.join(os.path.join(output_location),output_name+'.wav')
				elif output_name == None:
					now = datetime.now()
					timestamp = now.strftime("%m%d%H%M%S")
					output_name = timestamp
					wav_save_name = os.path.join(os.path.join(output_location),output_name+'.wav')
				
				wf = wave.open(wav_save_name, 'wb')
				wf.setnchannels(CHANNELS)
				wf.setsampwidth(self.p.get_sample_size(FORMAT))
				wf.setframerate(RATE)
				#framesToSave = b''.join(framesToSave)
				wf.writeframes(framesToSave)
				hasRead = True
				break
			else:
				readFrames += CHUNK_SIZE

		#close relevant things
		if hasRead:
			wf.close()
		#return success state
		if returnList != None:
			returnList[0] = hasRead
			returnList[1] = output_name
		#set doneRecording to True
		#self.recordLock.acquire()
		self.doneRecording = True
		#self.recordLock.release()
		return (hasRead,output_name)

	def get_next_tap_LIVE(self,stream,chunk_size,channels,threshold,rate,duration):
		data_read = stream.read(chunk_size)
		data_reader = StreamReader(data_read,channels)
		
		frames = ''

		crossed_thresh = False
		#check if any frame in chunk crossed threshold; if so, add proper duration for tap
		for i in range(0,data_reader.getnframes()):
			curr_frame = data_reader.readframes(1)
			unp_sign_val = unpack("<h",curr_frame[:2]) #this checks the first byte
			if abs(unp_sign_val[0]) > threshold:
				print 'CROSSED THRESH WITH VALUE: %s' % unp_sign_val[0]
				crossed_thresh = True
				print 'NEXT AVAILABLE INDEX: %s' % data_reader.getCurrentFrameIndex()
				break

		#if hasnt crossed thresh, return False
		if not crossed_thresh:
			print 'no tap found!'
			print data_reader.getnframes()
			return False,data_reader.getnframes()

		#otherwise, add the tap data
		required_frames = int(duration*rate)
		print "REQUIRED FRAMES: %s" % required_frames
		#frames_available = chunk_size*channels-crossed_index
		frames_available = data_reader.getnframes()-data_reader.getCurrentFrameIndex()
		print "FRAMES_AVAILABLE: %s" % frames_available
		still_to_obtain = required_frames-frames_available
		print "FRAMES TO GET: %s" % still_to_obtain
		if frames_available < required_frames:
			print 'LESS AVAILABLE THAN REQUIRED; GETTING MORE FRAMES'
			frames = data_reader.readframes(frames_available)
			print 'LENGTH OF FRAMES SO FAR: %s' % len(frames)
			additional_read = stream.read(still_to_obtain)
			data_reader = StreamReader(additional_read)
			print "ADDITIONAL DATA AVAILABLE: %s" % data_reader.getnframes()
			add_data = data_reader.readframes(still_to_obtain)
			print "ADDITIONAL DATA READ: %s bytes" % len(add_data)
			frames += add_data
		else:
			print 'ALL FRAMES AVAILABLE HERE'
			frames = data_reader.readframes(required_frames)

		print 'tap found!'
		print len(frames)
		return True,frames

	def stop(self):
		self.p.terminate()


class StreamReader():
	def __init__(self,data,channels=2,bits=16):
		self.INDEX = 0
		self.DATA = data
		self.CHANNELS = channels
		self.BITS = bits
		self.BYTESPERFRAME = (self.BITS/8)*self.CHANNELS

	def readframes(self,frameAmt):
		#if reached the end of data, return a blank string
		if self.INDEX == len(self.DATA):
			print 'ERROR: cant provide any more data'
			return ''
		try:
			frame = self.DATA[self.INDEX:self.INDEX+(self.BYTESPERFRAME*frameAmt)]
			self.INDEX += int(self.BYTESPERFRAME*frameAmt)
			if frameAmt > 1:
				print 'PROVIDING %s frames' % frameAmt
			return frame
		except IndexError:
			print 'ERROR: DATA STREAM OVERFLOW'
			frame = self.DATA[self.INDEX:]
			self.INDEX = len(self.DATA) #impossible index

	def getnframes(self):
		return int(len(self.DATA)/self.BYTESPERFRAME)

	def getCurrentFrameIndex(self):
		return (self.INDEX+1)/self.BYTESPERFRAME
