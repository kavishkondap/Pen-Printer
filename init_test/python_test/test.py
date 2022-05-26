import serial
import time

serialcomm = serial.Serial('COM5', 9600)
serialcomm.timeout = 0.3

while True:
	i = input('input(on #/off): ').strip()
	if i == 'done':
		print('finished')
		break
	serialcomm.write(i.encode())
	time.sleep(0.5)
	print(serialcomm.readline().decode())