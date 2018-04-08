import os
import socket
import time
from random import randint
import threading
import sys


BUFSIZE = 256
"""TOSEND = ("The aim of this project is to increase your familiarity with socket program"
	+ "ming, multi-threading, and the HTTP protocol. Your task is to write a basic "
	+ "HTTP server that responds correctly to a limited set of GET requests. The "
	+ "HTTP server must return valid response headers for a range of fles and "
	+ "paths. "
	+ "Your HTTP Server must be written in C. Submissions that do not compile "
	+ "and run on a NeCTAR instance may receive zero marks. You must write your "
	+ "own HTTP handling code, you may not use existing HTTP libraries.")"""
TOSEND = ("GET /hi.html HTTP/1.1\r\n")


def connect(addr):
	cli_addr = (socket.gethostname(), get_random_port())
	socket_obj = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM, proto=0)
	socket_obj.bind(cli_addr)
	print("Connecting to "+str(addr)+" from "+str(cli_addr), end=" ")
	socket_obj.connect(addr)
	print("Success!")
	return socket_obj


def mysend(conn, msg):
	totalsent = 0
	while totalsent<len(msg):
		chunk = bytes(msg.encode('ascii')[totalsent:])
		print("SENT CHUNK: "+chunk.decode('ascii'))
		sent = conn.send(chunk)
		totalsent += sent
	conn.shutdown(socket.SHUT_WR)


def myrecv(conn, msgs):
	while True:
		msg = str(conn.recv(BUFSIZE),'ascii')
		if len(msg) == 0:
			break
		print("RECIEVED CHUNK: "+msg)
		msgs.append(msg)
	return


def runclient(addr, msg):
	recd = []
	conn = connect(addr)
	sending = threading.Thread(target=mysend, args=(conn, msg))
	recving = threading.Thread(target=myrecv,args=(conn, recd))
	sending.start()
	recving.start()
	sending.join()
	recving.join()
	conn.close()
	return "".join(recd)


def get_random_port():
	return randint(49152,65535)


def main():
	host = str(sys.argv[1])
	port = int(sys.argv[2])
	if not host or not port:
		print("Incorrect input.")
	addr = (host, port)
	responses = runclient(addr, TOSEND)
	print("RESPONSES: "+responses)


if __name__ == '__main__':
	main()
