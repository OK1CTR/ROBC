# The class for PilsenCube bus message
class PilsenMsg:
	
	# Constructor needs Destination address (Byte), Command code (Byte) and the Payload data (ByteArray)
	def __init__(self, dest, cmd, data):
		# The packet destination address (8-bit number selected from the unit table)
		if dest < 0 or dest > 255: raise Exception('Wrong destination address put into a message constructor.')
		self.dest = dest.to_bytes(length=1, byteorder='little')
    # The command code (8-bit number) sent to the unit 
		self.cmd = cmd.to_bytes(length=1, byteorder='little')
		if cmd < 0 or cmd > 255: raise Exception('Wrong command code put into a message constructor.')
		# The packet payload without checksum
		self.payload = bytearray(data)

	# Returns the packet header in human readable format in String
	def header(self):
		s = unit_table.get('{0:02X}' . format(self.dest[0]), 'UNKNOWN')
		if self.cmd[0] >= 0x20 and self.cmd[0] < 0x80:
			c = self.cmd[0]
		else:
			c = ord('.') 
		return 'Dest: 0x{0:02X} ({1:s}) Cmd: 0x{2:02X} ({3:c})'.format(self.dest[0], s, self.cmd[0], c)
		
	# Returns the classic hex dump of the packet payload in String
	def hexdump(self):
		if (len(self.payload) == 0): return '0000: '
		dump = ''
		j = 0
		for i in self.payload:
			# row begin, address
			if j % 16 == 0:
				dump += '{0:04X}: '.format(j)
				s = ''
			# row content
			dump += '{0:02X} '.format(i)
			if i >= 0x20 and i < 0x80:  # readable part of ASCII
				s += '{0:c}'.format(i)
			else:
				s += '.' 
			# row end, ascii representation
			if (j + 1) % 16 == 0:
				dump += '{0:s}\n'.format(s)
				s = ''
			j += 1
		# ascii representation on unfinished line
		if s != '':
			dump += ' ' * (16 - (j % 16)) * 3
			dump += '{0:s}'.format(s)
		else:
			dump = dump.rstrip('\n')
		return dump

	# Initializes instance attributes from raw byte array
	def fromraw(self, cbuf):
		if len(cbuf) < 4: raise Exception('Wrong packet format, too short raw data array.') 
		dest = cbuf[0]; cmd = cbuf[1]
		# process the sum check
		sum = int(cbuf[-2:].decode(), 16)
		cbuf = cbuf[2:-2] # remove the dest. addr., cmd and chksum from the buffer
		sum += cmd
		for i in cbuf:
			sum += i
		sum %= 256
		if sum != 0: raise Exception('Wrong packet check sum.')
		# payload extraction and attribute setting
		pl = len(cbuf)
		if pl % 2 == 1: raise Exception('Wrong packet format, wrong length of raw data array.')
		pl //= 2
		self.dest = dest.to_bytes(length=1, byteorder='little')
		self.cmd = cmd.to_bytes(length=1, byteorder='little')
		self.payload = bytearray()
		for i in range(pl):
			a = int(cbuf[(i * 2):(i * 2 + 2)].decode(), 16)
			self.payload.append(a)
		return

	# Converts the instance attributes into the raw byte packet with the check sum.
	def toraw(self):
		cbuf = bytearray()
		cbuf.append(self.dest[0])
		cbuf.append(self.cmd[0])
		sum = self.cmd[0]
		for a in self.payload:
			s = '{0:02X}'.format(a)
			sum += ord(s[0]) + ord(s[1])
			cbuf.extend(s.encode())
		sum = -sum % 256
		s = '{0:02X}'.format(sum)
		cbuf.extend(s.encode())
		return cbuf

 	# Blocking packet receive from given serial port
	def listen_blk(self, port):
		cbuf = bytearray()
		while (1):
			c = port.read(1)
			if len(c) == 0: continue # is blocking
			a = int(c[0])
			if a != 0xAA:
				cbuf.append(a)
				continue
			break
		self.fromraw(cbuf)

  # Nonblocking receive of the packet from given serial port
	def listen_nblk(self, port):
		cbuf = bytearray()
		while (1):
			c = port.read(1)
			if len(c) == 0: break # non blocking
			a = int(c[0])
			if a != 0xAA:
				cbuf.append(a)
				continue
			break
		if len(c) != 0: self.fromraw(cbuf)

	# Sends the packet to the given serial port
	def talk(self, port):
		port.write(self.toraw())
		port.write(pils_term)


# The table of PilsenCUBE units destination addresses 
unit_table = {
	'CC': 'OBC-COM1',
	'D2': 'COM2',
	'B4': 'PWR',
	'E1': 'MES',
	'99': 'FREE-1',
	'87': 'FREE-2' }

# PilsenCUBE bus packet terminator
pils_term = b'\xAA'
