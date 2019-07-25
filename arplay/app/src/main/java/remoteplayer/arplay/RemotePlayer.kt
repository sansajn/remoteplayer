package remoteplayer.arplay

import android.app.Activity
import org.json.JSONArray
import org.json.JSONObject
import org.zeromq.ZContext
import org.zeromq.ZMQ
import remoteplayer.arplay.zmq.ZMQPushTask
import remoteplayer.arplay.zmq.ZMQSubscriberTask
import java.util.*

interface RemotePlayerListener {
	fun playProgress(position: Long, duration: Long, playlistId: Long, mediaIdx: Long, playbackState: Int, mode: Int)
	fun playlistContent(id: Long, items: List<String>)
}

class RemotePlayerClient(private val _activity: Activity) {

	fun connect(address: String, port: Int) {

		val subscriberTask = object : TimerTask() {
			override fun run() {
				_activity.runOnUiThread { ZMQSubscriberTask(this@RemotePlayerClient::processCommand).execute(_sub) }
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

	fun close() {
		_scheduler.cancel()
		_ctx.destroySocket(_push)
		_ctx.destroySocket(_sub)
		_ctx.destroy()
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

	fun registerListener(listener: RemotePlayerListener) {
		_listeners.put(listener, listener)
	}

	fun removeListener(listener: RemotePlayerListener) {
		_listeners.remove(listener)
	}

	private fun processCommand(cmd: String) {
		val json = JSONObject(cmd)
		when  (json.getString("cmd")) {
			"play_progress" -> handlePlayProgress(json)
			"playlist_content" -> handlePlaylistContent(json)
		}
	}

	private fun handlePlaylistContent(json: JSONObject) {
		val id = json.getLong("id")
		val items = toList(json.getJSONArray("items"))

		for (l in _listeners.values)
			l.playlistContent(id, items)
	}

	private fun handlePlayProgress(json: JSONObject) {
		val position = json.getLong("position")
		val duration = json.getLong("duration")
		val playlistId = json.getLong("playlist_id")
		val mediaIdx = json.getLong("media_idx")
		val playbackState = json.getInt("playback_state")
		val mode = json.getInt("mode")

		for (l in _listeners.values)
			l.playProgress(position, duration, playlistId, mediaIdx, playbackState, mode)
	}

	private fun toList(arr: JSONArray): MutableList<String> {
		val result = mutableListOf<String>()
		for (i in 0 until arr.length())
			result.add(arr.getString(i))
		return result
	}

	private val _ctx = ZContext()
	private val _sub = _ctx.createSocket(ZMQ.SUB)
	private val _push = _ctx.createSocket(ZMQ.PUSH)
	private val _pushQueue = mutableListOf<String>()  // ZMQ push socket queue
	private val _scheduler = Timer()
	private val _listeners = mutableMapOf<RemotePlayerListener, RemotePlayerListener>()
}
