#!/usr/bin/python

import zmq, time

DEFAULT_URL = 'https://www.youtube.com/watch?v=a01QQZyl-_I' 
PORT = 13333

def main():
	ctx = zmq.Context()

	subscriber = ctx.socket(zmq.SUB)
	subscriber.linger = 0
	subscriber.connect('tcp://localhost:%d' % (PORT, ))
	subscriber.setsockopt(zmq.SUBSCRIBE, '')  # set topic

	notifier = ctx.socket(zmq.PUSH)
	notifier.linger = 0
	notifier.connect('tcp://localhost:%d' % (PORT+2, ))

	time.sleep(0.1)  # ~100ms

	msg = {
		'cmd':'download',
		'url':DEFAULT_URL}

	notifier.send_json(msg)

	print('rplay << %s' % (msg,))

	print('done!')

if __name__ == '__main__':
	main()
