#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
Probe utility for exploring ZMQ messages send to arplay by rplay.

Dependencies:
	sudo apt-get install python3-zmq libzmq5
'''

import argparse, zmq, time, datetime, random

DEFAULT_PORT = 23333
DEFAULT_HOST = 'localhost'

def main(args):
	ctx = zmq.Context()
	
	subscriber = ctx.socket(zmq.SUB)
	subscriber.linger = 0
	subscriber.setsockopt_string(zmq.SUBSCRIBE, '')
	subscriber.connect('tcp://%s:%d' % (args.host, args.port,))

	requester = ctx.socket(zmq.DEALER)
	requester.linger = 0
	requester.connect('tcp://%s:%d' % (args.host, args.port+1, ))

	poller = zmq.Poller()
	poller.register(requester, zmq.POLLIN)
	poller.register(subscriber, zmq.POLLIN)

	print('listenning ...')

	while True:
		try:
			items = dict(poller.poll(100))  # 100ms poll
		except:
			break

		if subscriber in items:
			d = subscriber.recv_json()
			print('rplay (SUB) >> "%s"' % d)


		if requester in items:
			d = requester.recv_json()
			print('rplay (DEAL) >> "%s"' % d)

	print('done!')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Player Client probe options.')
	parser.add_argument('--port', type=int, default=DEFAULT_PORT, help='host port number')
	parser.add_argument('--host', default=DEFAULT_HOST, help='host IP address')
	args = parser.parse_args()
	
	main(args)
