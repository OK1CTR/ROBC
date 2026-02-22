import serial
import pilsenmsg as pm

SERIAL_PORT = 'COM5'
SERIAL_RATE = 250000

msgW = pm.PilsenMsg(0xCC, ord('A'), [0x00, 0x20, 0x0F])	
msgR = pm.PilsenMsg(0xCC, ord('A'), [0xFF, 0x20, 0xAA, 0x03])
msgResp = pm.PilsenMsg(0, 0, [])
ser = serial.Serial(SERIAL_PORT, SERIAL_RATE, timeout=1)

msgW.talk(ser)

i = 2
while (i > 0):
	msgResp.listen_blk(ser)
	print(msgResp.header())
	print(msgResp.hexdump())
	i -= 1

ser.close()
