package remoteplayer.arplay

import android.app.Activity
import org.json.JSONObject
import org.zeromq.ZContext
import org.zeromq.ZMQ
import java.util.*

interface RemotePlayerListener {
	fun playProgress(position: Long, duration: Long, playlistId: Long, mediaIdx: Long, playbackState: Int, mode: Int)
	fun playlistContent(id: Long, items: List<String>)
}

class RemotePlayerClient(private val _activity: Activity, private val _delegate: RemotePlayerListener) {

	fun connect(address: String, port: Int) {

		val subscriberTask = object : TimerTask() {
			override fun run() {
				_activity.runOnUiThread { ZMQSubscriberTask(_sub, _delegate).execute(null) }
			}
		}

		val pushTask = object : TimerTask() {
			override fun run() {
				if (_pushQueue.isNotEmpty()) {
					_activity.runOnUiThread {
						ZMQPushTask(_push, _pushQueue.toList()).execute()
						_pushQueue.clear()
					}
				}
			}
		}

		_sub.subscribe(ZMQ.SUBSCRIPTION_ALL)
		_sub.connect("$address:$port")

		_push.connect("$address:${port+2}")

		_scheduler.schedule(subscriberTask, 0, 100)
		_scheduler.schedule(pushTask, 0, 100)
	}

	fun clientReady() {
		val json = JSONObject()
		json.put("cmd", "client_ready")
		_pushQueue.add(json.toString())
	}

	fun seek(seconds: Int, media: String) {
		val json = JSONObject()
		json.put("cmd", "seek")
		json.put("position", seconds)
		json.put("media", media)
		_pushQueue.add(json.toString())
	}

	fun play(itemIdx: Int, playlistId: Long) {
		val json = JSONObject()
		json.put("cmd", "play")
		json.put("playlist", playlistId)
		json.put("idx", itemIdx)
		_pushQueue.add(json.toString())
	}

	fun pause() {
		val json = JSONObject()
		json.put("cmd", "pause")
		_pushQueue.add(json.toString())
	}

	fun stop() {
		val json = JSONObject()
		json.put("cmd", "stop")
		_pushQueue.add(json.toString())
	}

	private val _ctx = ZContext()
	private val _sub = _ctx.createSocket(ZMQ.SUB)
	private val _push = _ctx.createSocket(ZMQ.PUSH)
	private val _pushQueue = mutableListOf<String>()
	private val _scheduler = Timer()
}
