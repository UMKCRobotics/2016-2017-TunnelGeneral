import cv2
import numpy as np

cap = cv2.VideoCapture(0)

#img = cv2.imread('opencv_logo.png',0)
scale = 0.5

while(True):
	# Capture frame-by-frame
	ret = False
	ret = cap.grab()
	if ret:
		ret,frame = cap.retrieve()
		frame = cv2.resize(frame,(0,0),fx = scale, fy = scale)
		for n in range(0,5):
			frame = cv2.GaussianBlur(frame, (5, 5), 0)
		img = frame
		cimg = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

		circles = cv2.HoughCircles(cimg,cv2.HOUGH_GRADIENT,1,70,param1=50,param2=30,minRadius=60,maxRadius=1000)
		print circles
		if circles != None:
			circles = np.uint16(np.around(circles))
			for i in circles[0,:]:
				# draw the outer circle
				cv2.circle(cimg,(i[0],i[1]),i[2],(0,255,0),2)
				# draw the center of the circle
				cv2.circle(cimg,(i[0],i[1]),2,(0,0,255),3)

		cv2.imshow('detected circles',cimg)
	if cv2.waitKey(1) & 0xFF == ord('q'):
		break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()