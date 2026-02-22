import serial
import pilsenmsg as pm

SERIAL_PORT = 'COM4'
SERIAL_RATE = 250000

msg1 = pm.PilsenMsg(0xCC, ord('A'), [0x11, 0x12, 0x15, 0x17])	
msg2 = pm.PilsenMsg(0xD2, ord('B'), [0x21, 0x15, 0x75, 0x77])
msg3 = pm.PilsenMsg(0, 0, [])

#print(msg1.header())
#print(msg1.hexdump())
#print(msg2.header())
#print(msg2.hexdump())
#print('\n');

ser = serial.Serial(SERIAL_PORT, SERIAL_RATE, timeout=1)
#print(type(msg1.toraw()))
#print(msg1.toraw())
msg1.talk(ser)
#msg3.fromraw(msg1.toraw())
#print(msg3.header())
#print(msg3.hexdump())

i = 1
while (i > 0):
	msg3.listen_blk(ser)
	print(msg3.header())
	print(msg3.hexdump())
	i -= 1
ser.close()
