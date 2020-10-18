#!/usr/bin/env python3
# download progress test helper
import zmq, time, sys, argparse

DEFAULT_PORT = 23333
DEFAULT_DOWNLOAD_ITEM_NAME = 'faked music file'
DEFAULT_DOWNLOAD_COUNT = 1

arg_parser = argparse.ArgumentParser(
	description='Download progress simulator options.')
	
arg_parser.add_argument('--port', type=int, default=DEFAULT_PORT, help='port number')
arg_parser.add_argument('--name', default=DEFAULT_DOWNLOAD_ITEM_NAME, help='downloading item name')
arg_parser.add_argument('--count', type=int, default=DEFAULT_DOWNLOAD_COUNT, help='number of downloading items')


def main(args):
	ctx = zmq.Context()
	
	port = DEFAULT_PORT
	publisher = ctx.socket(zmq.PUB)
	publisher.bind('tcp://*:%d' % (port, ))
	
	responder = ctx.socket(zmq.ROUTER)
	responder.bind('tcp://*:%d' % (port+1, ))

	collector = ctx.socket(zmq.PULL)
	collector.bind('tcp://*:%d' % (port+2, ))

	download_count = args.count

	for n in range(1, download_count+1):
		if download_count > 1:
			item_name = args.name + ' %d' % n 
		else:
			item_name = args.name

		for p in range(5, 101, 5):
			progress_msg = {
				'cmd': 'download_progress',
				'items': [
					{'n': item_name, 'p': p}
				]
			}

			publisher.send_json(progress_msg)

			print(progress_msg)
			
			time.sleep(1.0)

	print('done!')
	

if __name__ == '__main__':
	args = arg_parser.parse_args()
	main(args)
