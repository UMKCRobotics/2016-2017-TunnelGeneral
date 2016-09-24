import numpy as np
import cv2
from pylab import array, uint8

cap = cv2.VideoCapture(1)

blank_white = None

def create_white(width,height,rgb_color=(255,255,255)):
	image = np.zeros((height,width,3), np.uint8)
	color = tuple(reversed(rgb_color))
	image[:] = color
	return image

def increase_contrast(img):
	phi = 1.0
	theta = 1.0
	maxIntensity = 255.0
	newimg = (maxIntensity/phi)*(img/(maxIntensity/theta))**0.5
	return array(newimg,dtype=uint8)

def sharpen_image(img):
	imgBlurred = cv2.GaussianBlur(img,(5,5),0)
	sharpened = cv2.addWeighted(img,1.5,imgBlurred,-0.5,0)
	return sharpened

def first_method(img):
    global blank_white
    imgSmall = img
    imgSmall = cv2.GaussianBlur(imgSmall,(5,5),0)
    imgSmall = cv2.GaussianBlur(imgSmall,(5,5),0)
    h_,w_ = imgSmall.shape[:2]
    if blank_white == None:
        blank_white = create_white(w_,h_,(255,255,255))
        blank_white = cv2.cvtColor(blank_white, cv2.COLOR_BGR2GRAY)

    imgSmall = cv2.addWeighted(imgSmall,0.9,blank_white,0.7,0)

    
    #imgSmall += 5
    for n in range(0,3):
        imgSmall = sharpen_image(imgSmall)
    sharpened = cv2.GaussianBlur(imgSmall,(5,5),0)
    #sharpened = increase_contrast(sharpened)
    sharpened = cv2.GaussianBlur(sharpened,(5,5),0)
    sharpened = cv2.erode(sharpened,None)

    ret1,thresh2 = cv2.threshold(sharpened,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
    return (thresh2,sharpened)


def second_method(img):
    global blank_white
    imgSmall = img
    unshaped = imgSmall.reshape(-1,1)
    unshaped = np.float32(unshaped)
    #do kmeans
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 10, 1.0)
    K = 2
    ret,label,center = cv2.kmeans(unshaped,K,None,criteria,10,cv2.KMEANS_RANDOM_CENTERS)
    #convert back
    center = np.uint8(center)
    res = center[label.flatten()]
    res2 = res.reshape((imgSmall.shape))
    cv2.imshow('kmeans',res2)

    imgSmall = cv2.GaussianBlur(res2,(5,5),0)
    imgSmall = cv2.GaussianBlur(imgSmall,(5,5),0)
    h_,w_ = imgSmall.shape[:2]
    if blank_white == None:
        blank_white = create_white(w_,h_,(255,255,255))
        blank_white = cv2.cvtColor(blank_white, cv2.COLOR_BGR2GRAY)

    imgSmall = cv2.addWeighted(imgSmall,0.9,blank_white,0.8,0)

    
    #imgSmall += 5
    for n in range(0,3):
        imgSmall = sharpen_image(imgSmall)
    sharpened = cv2.GaussianBlur(imgSmall,(5,5),0)
    #sharpened = increase_contrast(sharpened)
    sharpened = cv2.GaussianBlur(sharpened,(5,5),0)
    sharpened = cv2.erode(sharpened,None)

    ret1,thresh3 = cv2.threshold(sharpened,0,255,cv2.THRESH_BINARY+cv2.THRESH_OTSU)
    return (thresh3,sharpened)


def return_blob_img(img,imgBefore):
    imgSmall = img
    params = cv2.SimpleBlobDetector_Params()
    #the dots are circular
    params.filterByCircularity = 1
    params.minCircularity = 0.85
    #the dots are not elongated
    params.filterByInertia = 1
    params.minInertiaRatio = 0.5
    #the dots have some area constrictions
    params.filterByArea = 1
    params.minArea = 40


    detector = cv2.SimpleBlobDetector_create(params)
    blobs = detector.detect(thresh2)
    im_with_keypoints = cv2.drawKeypoints(imgBefore, blobs, np.array([]), (0,0,255), cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
        

    font = cv2.FONT_HERSHEY_SIMPLEX
    cv2.putText(im_with_keypoints,str(len(blobs)),(20,20),font,0.6,(0,0,255),2)
    return im_with_keypoints

while(True):
    # Capture frame-by-frame
    ret = False
    ret = cap.grab()
    if ret:
    	ret,frame = cap.retrieve()

    # Our operations on the frame come here
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    h,w = frame.shape[:2]
    #fraction = 5
    #imgSmall = gray[0+h/fraction:h-h/fraction,0+w/fraction:w-w/fraction] #limit what part of image is parsed
    thresh2,sharpened2 = first_method(gray)
    im_with_keypoints = return_blob_img(thresh2,sharpened2)
    #thresh3 = second_method(gray)
    #im_with_keypoints2 = return_blob_img(thresh3)
    #thresh2 = cv2.adaptiveThreshold(imgSmall,255,cv2.ADAPTIVE_THRESH_GAUSSIAN_C,cv2.THRESH_BINARY,11,2)
    #create blob detector
    ####set up detector params
    
    #cnts = cv2.findContours(gray.copy(), cv2.RETR_EXTERNAL,
	#			cv2.CHAIN_APPROX_SIMPLE)[-2]

    #K-means clustering for gray scale image, then do everything again

    # Display the resulting frame
    cv2.imshow('gray',gray)
    cv2.imshow('keypoints1',im_with_keypoints)
    #cv2.imshow('keypoints2',im_with_keypoints2)
    #laplacian = cv2.Laplacian(cv2.GaussianBlur(gray,(5,5),0),cv2.CV_16S)
    #laplacian = cv2.convertScaleAbs(laplacian)
    #cv2.imshow('laplacian',laplacian)
    #cv2.imshow('thresh1',thresh1)
    cv2.imshow('thresh2',thresh2)
    #cv2.imshow('thresh3',thresh3)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture
cap.release()
cv2.destroyAllWindows()