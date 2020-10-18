#!/usr/bin/env python3

import zmq, time

PORT = 23333

def main():
	ctx = zmq.Context()
	
	publisher = ctx.socket(zmq.PUB)
	publisher.bind('tcp://*:%d' % (PORT, ))
	
	responder = ctx.socket(zmq.ROUTER)
	responder.bind('tcp://*:%d' % (PORT+1, ))

	collector = ctx.socket(zmq.PULL)
	collector.bind('tcp://*:%d' % (PORT+2, ))

	for p in range(5, 101, 5):
		progress_msg = {
			'cmd': 'download_progress',
			'items': [
				{'n': 'faked music file', 'p': p}
			]
		}

		publisher.send_json(progress_msg)

		print(progress_msg)
		
		time.sleep(1.0)

	

if __name__ == '__main__':
	main()
