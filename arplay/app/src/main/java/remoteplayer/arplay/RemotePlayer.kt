package remoteplayer.arplay

import android.os.Handler
import android.os.Looper
import org.json.JSONArray
import org.json.JSONObject
import org.zeromq.ZContext
import org.zeromq.ZMQ
import remoteplayer.arplay.zmq.ZMQDealerRecvTask
import remoteplayer.arplay.zmq.ZMQDealerSendTask
import remoteplayer.arplay.zmq.ZMQPushTask
import remoteplayer.arplay.zmq.ZMQSubscriberTask
import java.util.*

interface PlaybackListener {

	fun playProgress(position: Long, duration: Long, playlistId: Long, mediaIdx: Long, playbackState: Int, mode: Int)
	fun playlistContent(id: Long, items: List<String>)
}

interface LibraryListener {

	fun mediaLibrary(items: List<String>)
}


class RemotePlayerClient {

	fun connect(address: String, port: Int) {

		_sub.subscribe(ZMQ.SUBSCRIPTION_ALL)
		_sub.connect("$address:$port")

		_dealer.connect("$address:${port+1}")

		_push.connect("$address:${port+2}")

		_scheduler.schedule(createSubscriberTask(), 0, 100)
		_scheduler.schedule(createDealerSendTask(), 0, 100)
		_scheduler.schedule(createDealerRecvTask(), 0, 100)
		_scheduler.schedule(createPushTask(), 0, 100)
	}

	fun close() {
		_scheduler.cancel()
		_ctx.destroySocket(_push)
		_ctx.destroySocket(_dealer)
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

	fun shuffle(value: Boolean) {
		val json = JSONObject()
		json.put("cmd", "playlist_shuffle")
		json.put("shuffle", value)
		_pushQueue.add(json.toString())
	}

	fun listMedia() {
		val json = JSONObject()
		json.put("cmd", "list_media")
		_dealerQueue.add(json.toString())
	}

	fun addToPlaylist(items: List<String>) {
		val json = JSONObject()
		json.put("cmd", "playlist_add")
		json.put("items", toJSONArray(items))
		_pushQueue.add(json.toString())
	}

	fun registerListener(listener: PlaybackListener) {
		_playbackListeners.put(listener, listener)
	}

	fun registerListener(listener: LibraryListener) {
		_libraryListeners.put(listener, listener)
	}

	fun removeListener(listener: PlaybackListener) {
		_playbackListeners.remove(listener)
	}

	fun removeListener(listener: LibraryListener) {
		_libraryListeners.remove(listener)
	}

	private fun processCommand(cmd: String) {
		val json = JSONObject(cmd)
		when  (json.getString("cmd")) {
			"play_progress" -> handlePlayProgress(json)
			"playlist_content" -> handlePlaylistContent(json)
		}
	}

	private fun processReplay(cmd: String) {
		val json = JSONObject(cmd)
		when (json.getString("cmd")) {
			"media_library" -> handleMediaLibrary(json)
		}
	}

	private fun handlePlaylistContent(json: JSONObject) {
		val id = json.getLong("id")
		val items = toList(json.getJSONArray("items"))

		for (l in _playbackListeners.values)
			l.playlistContent(id, items)
	}

	private fun handlePlayProgress(json: JSONObject) {

		val position = json.getLong("position")
		val duration = json.getLong("duration")
		val playlistId = json.getLong("playlist_id")
		val mediaIdx = json.getLong("media_idx")
		val playbackState = json.getInt("playback_state")
		val mode = json.getInt("mode")

		for (l in _playbackListeners.values)
			l.playProgress(position, duration, playlistId, mediaIdx, playbackState, mode)
	}

	private fun handleMediaLibrary(json: JSONObject) {
		val items = toList(json.getJSONArray("items"))
		for (l in _libraryListeners.values)
			l.mediaLibrary(items)
	}

	private fun toList(arr: JSONArray): MutableList<String> {
		val result = mutableListOf<String>()
		for (i in 0 until arr.length())
			result.add(arr.getString(i))
		return result
	}

	private fun toJSONArray(items: List<String>): JSONArray {
		val arr = JSONArray()
		items.forEach { arr.put(it) }
		return arr
	}

	private fun createSubscriberTask(): TimerTask {
		return object : TimerTask() {
			override fun run() {
				Handler(Looper.getMainLooper()).post {
					ZMQSubscriberTask(this@RemotePlayerClient::processCommand).execute(_sub)
				}
			}
		}
	}

	private fun createPushTask(): TimerTask {
		return object : TimerTask() {
			override fun run() {
				if (_pushQueue.isNotEmpty()) {
					Handler(Looper.getMainLooper()).post {
						ZMQPushTask(_push, _pushQueue.toList()).execute()
						_pushQueue.clear()
					}
				}
			}
		}
	}

	private fun createDealerSendTask(): TimerTask {
		return object : TimerTask() {
			override fun run() {
				if (_dealerQueue.isNotEmpty()) {
					Handler(Looper.getMainLooper()).post {
						ZMQDealerSendTask().execute(_dealer, _dealerQueue.toList())
						_dealerQueue.clear()
					}
				}
			}
		}
	}

	private fun createDealerRecvTask(): TimerTask {
		return object : TimerTask() {
			override fun run() {
				Handler(Looper.getMainLooper()).post {
					ZMQDealerRecvTask(this@RemotePlayerClient::processReplay).execute(_dealer)
				}
			}
		}
	}

	private val _ctx = ZContext()
	private val _sub = _ctx.createSocket(ZMQ.SUB)
	private val _dealer = _ctx.createSocket(ZMQ.DEALER)
	private val _push = _ctx.createSocket(ZMQ.PUSH)
	private val _pushQueue = mutableListOf<String>()  // ZMQ push socket (notify) queue
	private val _dealerQueue = mutableListOf<String>()  // ZMQ dealer (question) queue
	private val _scheduler = Timer()
	private val _playbackListeners = mutableMapOf<PlaybackListener, PlaybackListener>()
	private val _libraryListeners = mutableMapOf<LibraryListener, LibraryListener>()
}
