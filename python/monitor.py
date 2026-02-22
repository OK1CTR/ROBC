import serial
import sys
import pilsenmsg as pm
from tkinter import *
from tkinter.font import Font


# Button callback - EXIT
def cb_finish():
	global finish
	finish = 1


# Button callback - MESSAGE 1 TX
def cb_msg1():
	global rq_send
	rq_send = 1


# Button callback - MESSAGE 2 TX
def cb_msg2():
	global rq_send
	rq_send = 2


# Button callback - MESSAGE 3 TX
def cb_msg3():
	global rq_send
	rq_send = 3


# Button callback - MESSAGE 4 TX
def cb_msg4():
	global rq_send
	rq_send = 4


# Sent the message onto the PilsenCUBE bus
def msg_send():
	global cnt_msgtx
	if rq_send == 0: return
	cnt_msgtx += 1
	s_msgctx.set('{:04d}' . format(cnt_msgtx))
	if rq_send == 1:
		msg_1.talk(ser)
		return
	if rq_send == 2:
		msg_2.talk(ser)
		return
	if rq_send == 3:
		msg_3.talk(ser)
		return
	if rq_send == 4:
		msg_4.talk(ser)
		return
		
	
# ### MAIN ####
if __name__== "__main__":

	# default serial port number
	SERIAL_PORT = 4
	# serial port device prefix
	SERIAL_PORT_PREFIX = 'COM'
	# default baud rate
	SERIAL_RATE = 250000

	# program arguments - serial port number
	ser_port_num = SERIAL_PORT
	if len(sys.argv) > 1:
		try:
			ser_port_num = int(sys.argv[1])
		except:
			print('Specify a valid serial port number in the first program argument.')
	serial_port = '{0:s}{1:d}' . format(SERIAL_PORT_PREFIX, ser_port_num)
	try:
		ser = serial.Serial(serial_port, SERIAL_RATE, timeout = 0.1)
	except:
		print('Wrong serial port device {:s}.' . format(serial_port))	
	print('Listening on {:s} serial port.' . format(serial_port))

	# received message counter
	cnt_msgrx = 0
	# transmitted message counter
	cnt_msgtx = 0
	# program finish flag
	finish = 0
	# request to send a message
	rq_send = 0

	# messages to send
	msg_1 = pm.PilsenMsg(0xCC, ord('T'), [0x32, 0x33, 0x34, 0x35, 0x36])
	msg_2 = pm.PilsenMsg(0xCC, ord('T'), [0x32, 0x33, 0x34, 0x35, 0x36])
	msg_3 = pm.PilsenMsg(0xCC, ord('T'), [0x32, 0x33, 0x34, 0x35, 0x36])
	msg_4 = pm.PilsenMsg(0xCC, ord('T'), [0x32, 0x33, 0x34, 0x35, 0x36])
	# other defs
	msg_rx = pm.PilsenMsg(0, 0, [])
	tk = Tk()

	tk.title('PilsenCUBE monitor')
	fnt_mosp = Font(family='Monospace',size=12, weight='bold')

	# display
	disp_fm = Frame(tk)
	disp_fm.pack(side = TOP)
	s_msgcrx = StringVar()
	s_msgcrx.set('{:04d}' . format(cnt_msgrx))
	disp_msgcrx = Label(disp_fm, textvariable=s_msgcrx, font=fnt_mosp)
	disp_msgcrx.config(bg="black", fg="green", height = 1, width = 16)
	disp_msgcrx.pack(fill = X, side = LEFT)
	s_msgctx = StringVar()
	s_msgctx.set('{:04d}' . format(cnt_msgtx))
	disp_msgctx = Label(disp_fm, textvariable=s_msgctx, font=fnt_mosp)
	disp_msgctx.config(bg="black", fg="red", height = 1, width = 16)
	disp_msgctx.pack(fill = X, side = LEFT)

	# buttons
	but_msg1 = Button(tk, text ='Msg 1', command = cb_msg1)
	but_msg1.config(height = 1, width = 8)
	but_msg1.pack(fill = X, side = LEFT) 
	but_msg2 = Button(tk, text ='Msg 2', command = cb_msg2)
	but_msg2.config(height = 1, width = 8)
	but_msg2.pack(fill = X, side = LEFT) 
	but_msg3 = Button(tk, text ='Msg 3', command = cb_msg3)
	but_msg3.config(height = 1, width = 8)
	but_msg3.pack(fill = X, side = LEFT) 
	but_msg4 = Button(tk, text ='Msg 4', command = cb_msg4)
	but_msg4.config(height = 1, width = 8)
	but_msg4.pack(fill = X, side = LEFT) 
	but_finish = Button(tk, text ='Exit', command = cb_finish)
	but_finish.config(height = 1, width = 8)
	but_finish.pack(fill = X, side = LEFT) 

	# communication loop
	while (finish == 0):
		tk.update_idletasks()
		tk.update()
		msg_rx.listen_nblk(ser)
		if len(msg_rx.payload) == 0:
			if rq_send != 0: # free to send
				msg_send()
				rq_send = 0
			continue
		print(msg_rx.header())
		print(msg_rx.hexdump())
		msg_rx = pm.PilsenMsg(0, 0, [])
		cnt_msgrx += 1
		s_msgcrx.set('{:04d}' . format(cnt_msgrx)) 

	# finish
	print("Have a nice DOS!")
	ser.close()
