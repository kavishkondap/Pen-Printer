import cv2 as cv
import numpy as np
import math
import serial


def send_instructions(instructions):
	serialcomm = serial.Serial('COM3', 9600)
	serialcomm.timeout = 0.3
	total = len(instructions)
	progress = 0
	serialcomm.write("tell me you see meeeeeee (⓿_⓿)".encode())

	# wait for manual reset
	input("Press enter to start...")

	for instruction in instructions:
		# send instruction
		serialcomm.write(instruction.encode())
		print(instruction)

		# wait for response that it finished
		message = serialcomm.readline().decode()
		while message.startswith("executed:") == False:
			message = serialcomm.readline().decode()

		# show execution result
		print(message)

		# show progress
		progress += 1
		print("progress: " + str(math.floor(100*progress/total)) + "%")
		print("steps progress: " + str(progress) + "/" + "total")

	print("done... ✍(◔◡◔)")

	return


def make_instructions(frame):
    # defining variables
    margin = 0.5
    steps_per_inch = 2000/16

    width, height = frame.shape
    steps_per_pixel_horizontal = steps_per_inch * ((8.5 - (margin*2)) / width)
    steps_per_pixel_vertical = steps_per_inch * ((11 - (margin*2)) / height)
    steps_per_pixel_preserved = min(
        steps_per_pixel_horizontal, steps_per_pixel_vertical)

    total = np.sum([[1 if frame[x, y] == 0 else 0 for x in range(width)]
                    for y in range(height)])
    calculated = 0
    print(total)

    # calculating instructions
    global instructions
    global penHeight
    global penInitial
    global penFudge
    penHeight = 59
    penInitial = penHeight
    penFudge = 14
    minDiff = 15
    instructions = [
        "penSet " + str(penHeight) + " " + str(minDiff), "penUp", "aReset", "penUp"]

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
        global penHeight

        xGoalSteps = xPix * steps_per_pixel_preserved
        yGoalSteps = yPix * steps_per_pixel_preserved

        xDifSteps = math.floor(xGoalSteps - xSteps)
        yDifSteps = math.floor(yGoalSteps - ySteps)

        xSteps += xDifSteps
        ySteps += yDifSteps

        change = math.floor(x * penFudge/width)
        penGoal = penInitial + change
        # moves the pen up (first) if it is going out more
        if penGoal > penHeight:
            penHeight = penGoal
            instructions.append("penSet " + str(penHeight) + " " +
                                str(minDiff + math.floor(change)))

        # moves the pen to the next spot
        instructions.append("aMove 0," + str(xDifSteps * 16) +
                            ";0," + str(16*yDifSteps))

        # moves the pen up (second) if it is going out more
        if penGoal < penHeight:
            penHeight = penGoal
            instructions.append("penSet " + str(penHeight) + " " +
                                str(minDiff + math.floor(change)))

        # pressses the pen
        instructions.append("penPress")

        return

    visited = [[1 if c % 2 == 0 or r %
                2 == 0 else 0 for c in range(height)] for r in range(width)]
    global instructionWasMade
    instructionWasMade = True
    while calculated < total and instructionWasMade:
        instructionWasMade = False
        # find nearest undrawn pixel
        xOff = 0
        yOff = 0
        for num in range(1, max(x*2, (width-x)*2, y*2, (height-y)*2)):
            dir = -1 if num % 2 == 0 else 1
            checkY = y + yOff
            for dXOff in range(0, num):
                xOff += dir
                checkX = x + xOff
                # check out of bounds
                if checkX < 0 or checkX >= width or checkY < 0 or checkY >= height:
                    continue
                # check pixel
                if frame[checkX, checkY] == 0 and visited[checkX][checkY] == 0:
                    visited[checkX][checkY] = 1
                    x = checkX
                    y = checkY
                    stepTo(x, y)
                    calculated += 1
                    print("processing: " +
                          str(math.floor(100*calculated/total)) + "%")
                    instructionWasMade = True
                    break
            if instructionWasMade:
                break
            checkX = x + xOff
            for dYOff in range(0, num):
                yOff += dir
                checkY = y + yOff
                # check out of bounds
                if checkX < 0 or checkX >= width or checkY < 0 or checkY >= height:
                    continue
                # check pixel
                if frame[checkX, checkY] == 0 and visited[checkX][checkY] == 0:
                    visited[checkX][checkY] = 1
                    x = checkX
                    y = checkY
                    stepTo(x, y)
                    calculated += 1
                    print("processing: " +
                          str(math.floor(100*calculated/total)) + "%")
                    instructionWasMade = True
                    break
            if instructionWasMade:
                break

    # move pen back instruction
    instructions.append("aMove 0," + str(-xSteps*16) + ";0," + str(-ySteps*16))

    return instructions


def get_bw_frame():
    cap = cv.VideoCapture(0)

    while True:
		# get black and white image
        ret, frame = cap.read()
        gray = cv.cvtColor (frame, cv.COLOR_BGR2GRAY)
     
		# using a threshold to get image
        f2 = cv.adaptiveThreshold(
		gray, 255, cv.ADAPTIVE_THRESH_GAUSSIAN_C, cv.THRESH_BINARY, 9, 3)

        # img = cv.imread("mavy.png", cv.IMREAD_GRAYSCALE)

        cv.imshow("F2", f2)

		# letting user select frame to print with 'p'
        if cv.waitKey(20) == ord("p"):
            return f2


if __name__ == "__main__":
    frame = get_bw_frame()
    instructions = make_instructions(frame)
    # instructions = ["penSet 7 10", "penUp", "aMove 0,1000;0,1000", "aMove 0,-1000;0,-1000"]
    send_instructions(instructions)
