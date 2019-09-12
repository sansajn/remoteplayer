package remoteplayer.arplay

import android.util.Log

class Player {

	constructor(rplay: RemotePlayerClient) {
		_rplay = rplay
		_rplay.clientReady()
	}

	fun play(mediaIdx: Int) {
		if (_playlistId != 0L)
			_rplay.play(mediaIdx, _playlistId)
		else
			Log.d("player", "invalid playlist ID")
	}

	fun play() {
		play(_mediaIdx)
	}

	fun pauseResume() {
		_rplay.pause()
	}

	fun next() {
		if (_playlistId != 0L) {
			val idx = (_mediaIdx+1) % _playlistItems.size
			_rplay.play(idx, _playlistId)
		}
		else
			Log.d("player", "invalid playlist ID")
	}

	fun previous() {
		if (_playlistId != 0L) {
			var idx = _mediaIdx-1
			if (idx < 0)
				idx = _playlistItems.size-1
			_rplay.play(idx, _playlistId)
		}
		else
			Log.d("player", "invalid playlist ID")
	}

	fun seek(seconds: Int) {
		if (_mediaIdx != -1 && _playlistItems.isNotEmpty())
			_rplay.seek(seconds, currentMediaStr())
	}

	fun shuffle() {
		_shuffled = !_shuffled
		_rplay.shuffle(_shuffled)
	}

	fun playlist(): List<PlaylistItem> {
		return _playlistItems
	}

	fun playlistId(): Long {
		return _playlistId
	}

	fun currentMedia(): PlaylistItem {
		return _playlistItems[_mediaIdx]
	}

	fun currentMediaStr(): String {
		return if (_mediaIdx == -1 || _playlistItems.isEmpty())
			""
		else
			_playlistItems[_mediaIdx].id
	}

	fun currentMediaIdx(): Int {
		return _mediaIdx
	}

	// update methods
	fun updatePlaylist(playlistId: Long, items: List<PlaylistItem>) {
		if (_playlistId != playlistId) {
			_playlistItems = items.toMutableList()
			_playlistId = playlistId
		}
		else
			Log.d("player", "invalid playlist ID")
	}

	fun updateMediaIdx(idx: Int) {
		_mediaIdx = idx
	}

	fun updateShuffled(shuffled: Boolean) {
		_shuffled = shuffled
	}

	private val _rplay: RemotePlayerClient
	private var _shuffled = false
	private var _mediaIdx = -1  // -1 nothing selected
	private var _playlistId = 0L  // 0 invalid playlist ID
	private var _playlistItems = mutableListOf<PlaylistItem>()
}
