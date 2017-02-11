import cv2
import numpy as np

#import cv2
#from numpy import *
#from pylab import *
#from imgops import imutils
import math


def sharpen_image(img):
    imgBlurred = cv2.GaussianBlur(img, (5, 5), 0)
    sharpened = cv2.addWeighted(img, 1.5, imgBlurred, -0.5, 0)
    return sharpened


def invert_img(img):
	img = (255-img)
	return img


def canny(imgray):
	imgray = cv2.GaussianBlur(imgray, (11,11), 200)
	canny_low = 0
	canny_high = 100

	thresh = cv2.Canny(imgray,canny_low,canny_high)
	return thresh

def cnt_gui(img, contours):
	cnts = sorted(contours, key = cv2.contourArea, reverse = True)

	for i in range(0,len(cnts)):
		sel_cnts = sorted(contours, key = cv2.contourArea, reverse = True)[i]

		area = cv2.contourArea(sel_cnts)

		if area < 750:
			continue

		# get orientation angle and center coord
		center, axis,angle = cv2.fitEllipse(sel_cnts)

		hyp = 100  # length of the orientation line

		# Find out coordinates of 2nd point if given length of line and center coord 
		linex = int(center[0]) + int(math.sin(math.radians(angle))*hyp)
		liney = int(center[1]) - int(math.cos(math.radians(angle))*hyp)

		# Draw orienation
		cv2.line(img, (int(center[0]),int(center[1])), (linex, liney), (0,0,255),5)			 
		cv2.circle(img, (int(center[0]), int(center[1])), 10, (255,0,0), -1)

	return img

def filtering(imgray):
	#imgray = cv2.medianBlur(imgray, 11)
	#for n in range(0,1):
	#	imgray = sharpen_image(imgray)
	imgray = cv2.medianBlur(imgray, 11)
	#imgray = cv2.medianBlur(imgray, 11)
	#imgray = cv2.dilate(imgray, None, iterations=1)
	#for n in range(0,1):
	#	imgray = sharpen_image(imgray)
	#for n in range(0,2):
	#	imgray = cv2.GaussianBlur(imgray, (5, 5), 0)
	cv2.imshow('blurred',imgray)
	thresh = cv2.Canny(imgray,40,200)
	#retZ,thresh = cv2.threshold(imgray, 0, 255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)
	return thresh, imgray

cap = cv2.VideoCapture(0)

#img = cv2.imread('opencv_logo.png',0)
scale = 0.50

while(True):
	# Capture frame-by-frame
	ret = False
	ret = cap.grab()
	if ret:
		ret,frame = cap.retrieve()
		frame = cv2.resize(frame,(0,0),fx = scale, fy = scale)
		frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
		orig_frame = frame
		for n in range(0,5):
			frame = cv2.GaussianBlur(frame, (5, 5), 0)
		cimg = frame

		circles = cv2.HoughCircles(cimg,cv2.HOUGH_GRADIENT,1,70,param1=50,param2=30,minRadius=60,maxRadius=1000)
		print circles
		if circles != None:
			circles = np.uint16(np.around(circles))
			for i in circles[0,:]:
				# draw the outer circle
				cv2.circle(cimg,(i[0],i[1]),i[2],(0,255,0),2)
				# draw the center of the circle
				cv2.circle(cimg,(i[0],i[1]),2,(0,0,255),3)

		#cv2.imshow('detected circles',cimg)
		thresh, imgray = filtering(orig_frame)


		cv2.imshow('thresh image', thresh)
		__ , contours, hierarchy = cv2.findContours(thresh.copy(),cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)

		if len(contours) > 0:
			print contours[0]

		final = orig_frame.copy()
		# Iterate through all contours
		test = cnt_gui(final, contours)



		cv2.imshow('original image', orig_frame)
		cv2.imshow('test image', test)
	if cv2.waitKey(1) & 0xFF == ord('q'):
		break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()