package remoteplayer.arplay

import android.util.Log

class Player(private val _rplay: RemotePlayerClient) {

	fun play(itemIdx: Int) {
		if (_playlistId != 0L)
			_rplay.play(itemIdx, _playlistId)
		else
			Log.d("player", "invalid playlist ID")
	}

	fun pauseResume() {
		_rplay.pause()
	}

	fun next() {
		if (_playlistId != 0L) {
			val idx = (_mediaIdx+1) % _playlistItems.size
			_rplay.play(idx, _playlistId)
		}
	}

	fun previous() {
		if (_playlistId != 0L) {
			var idx = _mediaIdx-1
			if (idx < 0)
				idx = _playlistItems.size-1
			_rplay.play(idx, _playlistId)
		}
	}

	fun shuffle() {
		_shuffled = !_shuffled
		_rplay.shuffle(_shuffled)
	}

	// update methods
	fun updatePlaylist(playlist: List<PlaylistItem>, playlistId: Long) {
		if (_playlistId != playlistId) {
			_playlistItems = playlist.toMutableList()
			_playlistId = playlistId
		}
	}

	fun updateShuffled(shuffled: Boolean) {
		_shuffled = shuffled
	}

	private var _shuffled = false
	private var _mediaIdx = -1  // -1 nothing selected
	private var _playlistId = 0L
	private var _playlistItems = mutableListOf<PlaylistItem>()
}
