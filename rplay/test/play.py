#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
Probe utility for exploring ZMQ messages send to arplay by rplay.

Dependencies:
	sudo apt-get install python3-zmq libzmq5
'''

import argparse, zmq, time, datetime, random, json

DEFAULT_PORT = 23333
DEFAULT_HOST = 'localhost'

def request_list_media(req):
	'''expect response, use req.recv_multipart() to receive see interface API for response format'''
	req.send_json({"cmd":"list_media"})

def playlist_add(notifier, items):
	notifier.send_json({'cmd':'playlist_add', 'items':items})

def play(notfier, playlist_id, item_idx):
	notfier.send_json({'cmd':'play', 'playlist':playlist_id, 'idx':item_idx})

def main(args):
	ctx = zmq.Context()
	
	# to revceive news from server
	subscriber = ctx.socket(zmq.SUB)
	subscriber.linger = 0
	subscriber.setsockopt_string(zmq.SUBSCRIBE, '')
	subscriber.connect('tcp://%s:%d' % (args.host, args.port,))

	# to ask questions and receive answers
	requester = ctx.socket(zmq.DEALER)
	requester.linger = 0
	requester.connect('tcp://%s:%d' % (args.host, args.port+1, ))

	# push notifycations
	notifier = ctx.socket(zmq.PUSH)
	notifier.linger = 0
	notifier.connect(f'tcp://{args.host}:{args.port+2}')

	poller = zmq.Poller()
	poller.register(requester, zmq.POLLIN)
	poller.register(subscriber, zmq.POLLIN)
	poller.register(notifier, zmq.POLLIN)

	print('listenning ...')

	request_list_media(requester)

	while True:
		try:
			items = dict(poller.poll(100))  # 100ms poll
		except:
			break

		if subscriber in items:
			d = subscriber.recv_json()
			print('rplay (news) >> "%s"' % d)

			if 'cmd' in d:
				if d['cmd'] == 'playlist_content':
					# we have playlist, we can send play
					items = d['items']
					assert len(items) > 0, "expect not empty playlist"
					playlist_id = d['id']
					play(notifier, playlist_id, 0)

		if requester in items:
			d = requester.recv_multipart()  # list of bytes
			res = json.loads(d[-1])  # assume last frame is json
			if 'cmd' in res:
				if res['cmd'] == 'media_library':
					items = res['items']
					print(f'rplay (response) >> media_library with {len(items)} items')
					# we have library and now can add to playlist one item
					playlist_add(notifier, [items[0]])
					# now we need to wait for playlist_content
				else:
					print(f'rplay (response) >> unknown response (cmd={res["cmd"]})')
			else:
				print('rplay (response) >> "%s"' % d)

	print('done!')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Player Client probe options.')
	parser.add_argument('--port', type=int, default=DEFAULT_PORT, help='host port number')
	parser.add_argument('--host', default=DEFAULT_HOST, help='host IP address')
	args = parser.parse_args()
	
	main(args)
