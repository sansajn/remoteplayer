'''
clone client stub 
implements: heart-beat, request-for-registration, request-api-version, 
	set-position-vector
'''
import zmq, time, datetime, random

def main():
	ctx = zmq.Context()
	subscriber = ctx.socket(zmq.SUB)
	subscriber.linger = 0
	subscriber.setsockopt(zmq.SUBSCRIBE, '')
	subscriber.connect('tcp://localhost:3333')

	requester = ctx.socket(zmq.DEALER)
	requester.linger = 0
	requester.connect('tcp://localhost:3334')
	
	publisher = ctx.socket(zmq.PUSH)
	publisher.linger = 0
	publisher.connect('tcp://localhost:3335')

	random.seed()
	whoami = random.randint(0, 1e9)

	# log in
	publisher.send_json({'Cmd':'RequestForRegistration', 'WhoAmI':whoami})

	# ask for api version
	requester.send_json({'Cmd': 'RequestApiVersion'})

	poller = zmq.Poller()
	poller.register(requester, zmq.POLLIN)
	poller.register(subscriber, zmq.POLLIN)

	heart_beat_trigger = time.time() + 5

	while True:
		try:
			items = dict(poller.poll(100))  # 100ms poll
		except:
			break

		if subscriber in items:
			d = subscriber.recv_json()
			if d['Cmd'] == 'SetPositionVector':
				posobj = d['PosObject']
				print('position %s received' % posobj)
			elif d['Cmd'] == 'ShowContent':
				url = d['Object']
				print('show-content "%s" command received' % url)
			elif d['Cmd'] == 'HideContent':
				print('hide-content command received')
			elif d['Cmd'] == 'StartRestart':
				print('start-restart command received')
			elif d['Cmd'] == 'Pause':
				print('pause comand received')
			elif d['Cmd'] == 'SetUrl':
				print('set-url %s command received' % d['CurrentUrl'])
			else:
				print('unhandled command "%s" received' % d)


		if requester in items:
			d = requester.recv_json()
			if 'ApiVersion' in d:
				print('server api version: %s' % d['ApiVersion'])

		if heart_beat_trigger < time.time():
			ftime = datetime.datetime.utcnow().strftime('%Y-%m-%d-%H-%M-%S-%f')
			publisher.send_json({'Cmd':'HeartBeat', 'UtcTimestamp':ftime, 'WhoAmI':whoami})
			heart_beat_trigger = time.time() + 5

	print('done!')

if __name__ == '__main__':
	main()
