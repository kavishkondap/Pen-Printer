from calendar import c
import cv2 as cv
import numpy as np
import math
import serial
import time
# from requests import get


def send_instructions(instructions):
	serialcomm = serial.Serial('COM7', 9600)
	serialcomm.timeout = 0.3
	total = len(instructions)
	progress = 0
	serialcomm.write("tell me you see meeeeeee (⓿_⓿)".encode())
	time.sleep(30)
	for instruction in instructions:
		#send instruction
		serialcomm.write(instruction.encode())
		print(instruction)
		

		#wait for response that it finished
		message = serialcomm.readline().decode()
		while message.startswith("executed:") == False:
			message = serialcomm.readline().decode()

		#show execution result
		print(message)

		#show progress
		progress+=1
		print("progress: " + str(math.floor(100*progress/total)) + "%")

	print("done... ✍(◔◡◔)")

	return

def make_instructions(frame):
	# defining variables
	margin = 0.5
	steps_per_inch = 2000/16

	width, height = frame.shape
	steps_per_pixel_horizontal = steps_per_inch * ((8.5 - (margin*2)) / width)
	steps_per_pixel_vertical = steps_per_inch * ((11 - (margin*2)) / height)

	total = np.sum([[1 if frame[x, y] == 0 else 0 for x in range(width)] for y in range(height)])
	calculated = 0
	print(total)

	# calculating instructions
	global instructions
	instructions = ["penSet 7", "penUp"] # TODO: add aReset
	global x
	global y
	global xSteps
	global ySteps
	x = 0
	y = 0
	xSteps = 0
	ySteps = 0

	def stepTo(xPix, yPix):
		global xSteps
		global ySteps
		
		xGoalSteps = xPix * steps_per_pixel_horizontal
		yGoalSteps = yPix * steps_per_pixel_vertical

		xDifSteps = math.floor(xGoalSteps - xSteps)
		yDifSteps = math.floor(yGoalSteps - ySteps)

		instructions.append("aMove 0," + str(xDifSteps * 16) + ";0," + str(16*yDifSteps))
		instructions.append("penPress")

		xSteps+=xDifSteps
		ySteps+=yDifSteps

		return

	visited = [[0 for c in range(height)] for r in range(width)]
	global instructionWasMade
	instructionWasMade = True
	while calculated < total and instructionWasMade:
		instructionWasMade = False
		# find nearest undrawn pixel
		xOff = 0
		yOff = 0
		for num in range(1, max(x, width-x, y, height-y)):
			dir = -1 if num%2==0 else 1
			checkY = y + yOff
			for dXOff in range(0, num):
				xOff+=dir
				checkX = x + xOff
				#check out of bounds
				if checkX < 0 or checkX >= width or checkY < 0 or checkY >= height:
					continue
				#check pixel
				if frame[checkX, checkY] == 0 and visited[checkX][checkY] == 0:
					visited[checkX][checkY] = 1
					x = checkX
					y = checkY
					stepTo(x, y)
					calculated+=1
					print("processing: " + str(math.floor(100*calculated/total)) + "%")
					instructionWasMade = True
					break
			if instructionWasMade:
				break
			checkX = x + xOff
			for dYOff in range(0, num):
				yOff+=dir
				checkY = y + yOff
				#check out of bounds
				if checkX < 0 or checkX >= width or checkY < 0 or checkY >= height:
					continue
				#check pixel
				if frame[checkX, checkY] == 0 and visited[checkX][checkY] == 0:
					visited[checkX][checkY] = 1
					x = checkX
					y = checkY
					stepTo(x, y)
					calculated+=1
					print("processing: " + str(math.floor(100*calculated/total)) + "%")
					instructionWasMade = True
					break
			if instructionWasMade:
				break

	#move pen back instruction
	instructions.append("aMove " + str(-xSteps) + "," + str(-ySteps) + ";0,0")

	return instructions

def get_bw_frame():
    cap = cv.VideoCapture(0)

    global canny_p1
    global canny_p2
    canny_p1 = 50
    canny_p2 = 200

    def set_canny_p1(value):
        global canny_p1
        canny_p1 = value

    def set_canny_p2(value):
        global canny_p2
        canny_p2 = value

    global first
    first = True

    while True:
        # get black and white image
        ret, frame = cap.read()
        gray = cv.cvtColor(frame, cv.COLOR_BGR2GRAY)

        # blur it to get rid of extraneous edges
        gray = cv.medianBlur(gray, 3)

        # using a threshold to get image
        f2 = cv.adaptiveThreshold(
            gray, 255, cv.ADAPTIVE_THRESH_GAUSSIAN_C, cv.THRESH_BINARY, 9, 3)

        # using edge detection alg
        frame = cv.Canny(frame, canny_p1, canny_p2)
        frame = cv.bitwise_not(frame)

        # showing
        cv.imshow("Display", frame)
        cv.imshow("F2", f2)

        # make trackbars
        if first:
            first = False
            cv.createTrackbar('CanP1', "Display", 0, 100, set_canny_p1)
            cv.createTrackbar('CanP2', "Display", 0, 1000, set_canny_p2)

        # letting user select frame to print with 'p'
        if cv.waitKey(200) == ord("p"):
            return f2

if __name__ == "__main__":
	frame = get_bw_frame()
	instructions = make_instructions(frame)
	# instructions = ["penSet 7", "penUp", "aMove 0,1000;0,1000", "aMove 0,-1000;0,-1000"]
	send_instructions(instructions)
