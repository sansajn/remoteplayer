#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
Probe utility for exploring ZMQ messages send to arplay by rplay.

Tested under: Ubuntu 24.04 LTS

Dependencies:
	sudo apt-get install python3-zmq libzmq5
'''

import argparse, json
import zmq

DEFAULT_PORT = 23333
DEFAULT_HOST = 'localhost'

def stop(notifier):
	cmd = {'cmd':'stop'}
	notifier.send_json(cmd)
	print(f'rplay << {cmd}')

def main(args):
	ctx = zmq.Context()

	# socket to push notifications
	notifier = ctx.socket(zmq.PUSH)
	notifier.linger = 0
	notifier.connect(f'tcp://{args.host}:{args.port+2}')

	stop(notifier)

	print('done!')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Player Client probe options.')
	parser.add_argument('--port', type=int, default=DEFAULT_PORT, help='host port number')
	parser.add_argument('--host', default=DEFAULT_HOST, help='host IP address')
	args = parser.parse_args()

	main(args)
