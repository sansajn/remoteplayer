package remoteplayer.arplay.zmq

import android.os.AsyncTask
import org.json.JSONArray
import org.json.JSONObject
import org.zeromq.ZMQ
import remoteplayer.arplay.RemotePlayerListener

class ZMQSubscriberTask(private val _sub: ZMQ.Socket, private val _delegate: RemotePlayerListener) : AsyncTask<Void, Void, Void?>() {

	override fun doInBackground(vararg params: Void?): Void? {
		val d = _sub.recvStr(ZMQ.NOBLOCK)
		if (d != null)
			_commands.add(d)
		return null
	}

	override fun onPostExecute(result: Void?) {
		super.onPostExecute(result)

		for (cmd in _commands) {
			val json = JSONObject(cmd)
			when  (json.getString("cmd")) {
				"play_progress" -> handlePlayProgress(json)
				"playlist_content" -> handlePlaylistContent(json)
			}
		}

		_commands.clear()  // flush commands
	}

	private fun handlePlaylistContent(json: JSONObject) {
		val id = json.getLong("id")
		val items = toList(json.getJSONArray("items"))
		_delegate.playlistContent(id, items)
	}

	private fun handlePlayProgress(json: JSONObject) {
		val position = json.getLong("position")
		val duration = json.getLong("duration")
		val playlistId = json.getLong("playlist_id")
		val mediaIdx = json.getLong("media_idx")
		val playbackState = json.getInt("playback_state")
		val mode = json.getInt("mode")
		_delegate.playProgress(position, duration, playlistId, mediaIdx, playbackState, mode)
	}

	private fun toList(arr: JSONArray): MutableList<String> {
		val result = mutableListOf<String>()
		for (i in 0..arr.length()-1) {
			result.add(arr.getString(i))
		}
		return result
	}

	private var _commands = mutableListOf<String>()
}
