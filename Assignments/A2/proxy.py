#!/usr/bin/python

import socket
import sys
import threading
import thread
import time
import socket               
import subprocess
import shlex
import os
import sys
import errno
import itertools
import os
import re
import datetime

#___________________________________________________________________________________________________
#-------------------------------------------Constants-----------------------------------------------

BUFFER_SIZE = 4096 # byte buffer

# Boolean values to see which logOptionis used
STRIP = False
HEX = False
AUTON = False
RAW = False

# Default [-logOptions]
_RAW_ = '-raw'
_STRIP_ = '-strip'
HEX_STR = '-hex'
_AUTO_N = '-auto'
N_VALUE = ''

# __________________________________________________________________________________________________
#------------------------------------------- Utility functions--------------------------------------

# Function to parse input data for -autoN
def parseDataAutoN(autoN, data, prefix):
	printData = ""
	for i in range (0, len(data), autoN):
		for x in (repr(data)[i:i+autoN]):
			if (ord(x)>=32 & ord(x)<= 127):
				printData += x
			elif (ord(x) < 32):
				printData += repr(x)
			else:
				printData += '\\'+hex(x)

	print (prefix + ''.join(l + ('\n' + prefix) * (n % autoN == (autoN-1)) for n, l in enumerate(printData)))

# Function to change input into list of corresponing ints 
def inputCharToInt(prefix,input_values):
    rawLog = map(lambda c: ord(c), input_values) # Apply function to every item of iterable and return a list of the results (ord of character)
    resulting_string = prefix + rawLog # add prefix to raw info Log
    return resulting_string

# Function to change prefix into list of corresponding ints
def prefixCharToInt(prefix):
    pre = map(lambda c: ord(c), prefix) 
    return pre

# Function to return raw data with prefix in each line
def rawOutputGenerator(prefix,input_data):
    outVal = addPrefixString(input_data,prefix)
    outVal = ''.join(str(chr(x)) for x in outVal)
    return outVal

# Function to get rid of non printable characters and replace them with '.'
def stripOutputGenerator(prefix,input_data):
    outVal = addPrefixString(input_data,prefix)
    char = ''
    for x in outVal:
		if (charConverter(x) == 10):
			char += '.\n'
		else:
    			char += chr(charConverter(x))
    outVal = ''.join(str(char))
    return outVal

# Function to check for non printable characters, exception is '\n'
def charConverter(inputInt):
    if (inputInt == 10):
	return 10
    elif((inputInt < 32) or(inputInt > 127)):
        return  46
    else:
        return inputInt

# Function to convert character to hex
def hexConverter(input_data):
    resulting_string = map(lambda c: hex(ord(c))[2:],input_data)
    for i,j in enumerate(resulting_string):
       if len(j) < 2:
          resulting_string[i] = '0' + j
    return resulting_string       

# Function to insert prefix into input
def addPrefixString(inputList, prefix):
    prefix.reverse() # Reverse prefix list
    end = len(inputList) # Get length of input list
    for i,j in enumerate(inputList): # Enumerate through the input list
       if (j == 10 and i != end-1): # If '\n' and not the end of the input list
           map(lambda k: inputList.insert(i+1,k), prefix) # map anonymous function over prefix 
    return inputList 
    
# Function to parse the input parameters and set appropriate booleans
def parseLogOptions():
    try:
        arg = sys.argv[1] 
        if (str(arg)[0:4] == HEX_STR):
            global HEX
            HEX = True
        elif(str(arg)[0:4] == _RAW_):
            global RAW 
            RAW = True
        elif (str(arg)[0:5] == _AUTO_N):
            global AUTON
            AUTON = True
            global N_VALUE
            N_VALUE = int(arg[5:])         
        elif(str(arg)[0:6] == _STRIP_):
            global  STRIP
            STRIP = True
    except Exception as e:
        print("Exception: " + str(e))

# Referenced https://gist.github.com/7h3rAm/5603718
def hexdump(src, prefix, length=16):
    result = []
    digits = 4 if isinstance(src, unicode) else 2

    for i in xrange(0, len(src), length):
       s = src[i:i+length]
       hexa = ' '.join(["%*X" % (digits, ord(x))  for x in s])
       text = ''.join([x if 0x20 <= ord(x) < 0x7F else b'.'  for x in s])
       if (i == 0):
	 result.append( "\n---> %08X   %-*s   %s" % (i, length*(digits + 1), hexa, text) )
       else:
	 result.append( "%08X   %-*s   %s" % (i, length*(digits + 1), hexa, text) )
    if '---> ' == prefix:
    	print '\n---> '.join(result)
    else:
	print '\n<--- '.join(result)  

# ____________________________________________________________________________________________________________________
#--------------------------------------------------Main Functions-----------------------------------------------------  

   
# Referenced from: http://www.techbeamers.com/python-tutorial-write-multithreaded-python-server/
class ClientThread ( threading.Thread):

   # Overriden the Thread's __init__ 
   def __init__ ( self, channel, address, receiver, threads, prefix ):

      self.channel = channel # new socket object
      self.address = address # address bound to the socket
      self.receiver = receiver 
      self.threads = threads # Threads
      self.prefix = prefix # Which prefix to use
      threading.Thread.__init__ ( self )

   def run ( self ):
        cont = True # Boolean to see if we are done with the connection
        try:
            while cont:                                     
                servResp = self.channel.recv(BUFFER_SIZE) # Receive info
                self.receiver.send(servResp) # Send the info to the other machine
                logger(servResp, self.prefix) # log the information
                if not servResp: # If no response
                    # Close the thread and set boolean to False
       		    self.receiver.close()
                    self.channel.close()
                    cont = False                    
                                                          
        except socket.error as t:
            if t.errno == errno.EPIPE:
                print("Client Closed")
                cont = False
                self.receiver.send(bytearray("Broken Pipe"))                        

# main function
# Set -logOption parameters etc.
# Start the server
def main():
    parseLogOptions()
    clientListener()    

# Depending on which logOption chosen, parse and log messages accordingly
def logger(input_values, prefix):
    if(RAW):
        pre = prefixCharToInt(prefix) # returns list of ints for prefix
        resulting_string = inputCharToInt(pre,input_values) # returns list of ints for data
        resulting_string = rawOutputGenerator(pre,resulting_string)
        print(resulting_string)
    elif(HEX):
        hexdump(input_values, prefix)   
    elif(STRIP):
        pre = prefixCharToInt(prefix)
        resulting_string = inputCharToInt(pre,input_values)
        resulting_string = stripOutputGenerator(pre,resulting_string) 
	print resulting_string        
    elif(AUTON):
        parseDataAutoN(N_VALUE,input_values,prefix)

# Called from main function. You can call this our real main fuction to deal with the connections, threads, etc.
def clientListener():
   ip = 'localhost' # Always use localhost
   
   client_Socket = socket.socket(socket.AF_INET,socket.SOCK_STREAM) # Create a socket object
   client_Socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) # Make sure the connection doesn't hang   
   
   if(len(sys.argv) == 5): # If we have [-logOption] included
       sourcePort = int(sys.argv[2])        
       destHost = sys.argv[3]         
       destPort = int(sys.argv[4])     
   else:
       sourcePort = int(sys.argv[1]) 
       destHost = sys.argv[2]         
       destPort = int(sys.argv[3])

# For a bit of that beautiful ASCII art
   with open("img", "r") as file:
	print file.read()
   
   print ("[+] Port logger running: srcPort = "+str(sourcePort)+" host = "+str(destHost)+" dstPort = "+str(destPort)) # Start of the proxy logger       
   client_Socket.bind(('', sourcePort)) # Bind to the port
   client_Socket.listen(5) # Wait for client connection. (5) - multiple clients allowed                                
   threads = [] # Thread management   
   while True: # Program should always run 
      conn, addr = client_Socket.accept() # Establish connection with client         
      server_Socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # The AF_* and SOCK_* constants are AddressFamily and SocketKind IntEnum collections.
      server_Socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR,1) # Set the value of the given socket option
      server_Socket.connect((destHost,destPort)) # Open a TCP connection to hostname on the port
      server_Socket.send('') 
      
      print ("[+] New connection: " + str(datetime.datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S")) +", from "+ str(addr[0])) # Program output as per requirements    
      try:
         client_Thread = ClientThread(conn, (ip, sourcePort), server_Socket, threads, '<--- ') # Create new client thread, pass appropriate arguments
         client_Thread.start() # Start the thread
         threads.append(client_Thread) # Put thread into thread list
         server_Thread = ClientThread(server_Socket, (destHost,destPort), conn, threads,'---> ') # Create new server thread, pass appropriate arguments
         server_Thread.start() #Start the server thread
         threads.append(server_Thread) # Put thread into thread list
      except Exception as b:
          print("Exception b =" + str(b)) # In case something goes wrong
          server_Socket.close()

if __name__ == '__main__':
    main()
