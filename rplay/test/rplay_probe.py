'''
PC (Player Client) probe utility for exploring ZMQ messages send to PC by PM (Player Manager).
'''
import argparse, zmq, time, datetime, random

def main(args):
	#if len(args.filter) > 0:
	#	print('filtered commands: %s' % args.filter)

	ctx = zmq.Context()
	subscriber = ctx.socket(zmq.SUB)
	subscriber.linger = 0
	subscriber.setsockopt(zmq.SUBSCRIBE, '')
	subscriber.connect('tcp://localhost:%d' % (args.port,))
	requester = ctx.socket(zmq.DEALER)
	requester.linger = 0
	requester.connect('tcp://localhost:%d' % (args.port+1, ))

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
			#if d['Cmd'] not in args.filter:
			#	print('subscriber >> "%s"' % d)
			print('subscriber >> "%s"' % d)


		if requester in items:
			d = requester.recv_json()
			#if d['Cmd'] not in args.filter:
			#	print('requester >> "%s"' % d)
			print('requester >> "%s"' % d)

	print('done!')

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Player Client probe options.')
	parser.add_argument('--port', type=int, default=3333, help='ZMQ channel port number')
	#parser.add_argument('--filter', nargs='*', default=[], help='filter out specified commands')
	args = parser.parse_args()
	
	main(args)
