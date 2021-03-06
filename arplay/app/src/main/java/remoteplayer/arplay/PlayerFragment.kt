package remoteplayer.arplay

import android.arch.lifecycle.ViewModelProviders
import android.os.Bundle
import android.support.v4.app.Fragment
import android.text.format.DateUtils
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.SeekBar
import android.widget.Toast
import kotlinx.android.synthetic.main.fragment_player.*
import kotlinx.android.synthetic.main.fragment_player.view.*
import java.io.File
import java.util.*

class PlayerFragment : Fragment(), PlaybackListener {

	override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
		super.onViewCreated(view, savedInstanceState)

		val viewModel = ViewModelProviders.of(requireActivity()).get(MainViewModel::class.java)
		_rplayClient = viewModel.remotePlayerClient()
		_rplayClient.registerListener(this)
		_rplayClient.clientReady()

		_scheduler.schedule(createPlaybackStoppedTask(), 100, 500)
	}

	override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
		val view = inflater.inflate(R.layout.fragment_player, container, false)

		view.playlist_items.emptyView = view.empty

		view.playlist_items.adapter = PlaylistAdapter(requireContext(), listOf())
//		dummyContent(view)

		view.playlist_items.onItemClickListener = AdapterView.OnItemClickListener { parent, view, position, id ->
			askPlay(position)
		}

		view.shuffle.setOnClickListener { askShuffle() }
		view.previous.setOnClickListener { askPrevious() }
		view.next.setOnClickListener { askNext() }

		view.timeline.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
			override fun onStopTrackingTouch(seekBar: SeekBar?) {
				if (seekBar != null) {
					val media = currentMedia()
					if (media.isNotEmpty()) {
						val seconds = ((seekBar.progress).toDouble() / 100.0 * _duration.toDouble() / 1000000000.0).toInt()
						_rplayClient.seek(seconds, media)
					}
					else {
						Toast.makeText(requireActivity(), "unknown played item, seek ignored", Toast.LENGTH_SHORT).show()
					}
				}
			}

			override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {}
			override fun onStartTrackingTouch(seekBar: SeekBar?) {}
		})

		return view
	}

	override fun onDestroyView() {
		super.onDestroyView()
		_scheduler.cancel()
		_rplayClient.removeListener(this)
	}

	override fun playlistContent(id: Long, items: List<String>) {
		_playlistId = id

		_playlistItems.clear()
		for (item in items)
			_playlistItems.add(toPlaylistItem(item))

		playlist_items.adapter = PlaylistAdapter(requireContext(), _playlistItems)
	}

	override fun playProgress(position: Long, duration: Long, playlistId: Long, mediaIdx: Long, playbackState: Int, mode: Int) {

		if (playlistId != _playlistId)
			return  // playlist not match, wait for playlist_content message first

		_position = position
		_duration = duration

		time.text = formatDuration(position / 1000000000)
		length.text = formatDuration(duration / 1000000000)
		timeline.progress = (position.toDouble() / duration.toDouble() * 100.0).toInt()

		if (_mediaIdx != mediaIdx.toInt()) {
			val currentItem = _playlistItems[mediaIdx.toInt()]
			current_title.text = currentItem.title
			current_artist.text = currentItem.artist
			_mediaIdx = mediaIdx.toInt()
		}

		if (!_isPlaying && playbackState == 1) {  // playing
			_isPlaying = true
			onPlaybackPlay()
		}
		else if (!_isPlaying && playbackState == 2) {  // paused
			_isPlaying = true
			onPlaybackStop()
		}

		_shuffled = (mode == 1)

		_lastPlayProgressStamp = System.currentTimeMillis()
	}

	// events
	private fun onPlaybackStop() {
		play_pause.setImageResource(R.drawable.ic_baseline_play_arrow_24px)
		play_pause.setOnClickListener {
			if (_isPlaying)
				askPauseResume()
			else
				askPlay(_mediaIdx)
			onPlaybackPlay()
		}
	}

	private fun onPlaybackPlay() {
		play_pause.setImageResource(R.drawable.ic_baseline_pause_24px)
		play_pause.setOnClickListener {
			askPauseResume()
			_isPlaying = false
			onPlaybackStop()
		}
	}

	private fun askPlay(itemIdx: Int) {
		if (_playlistId != 0L)
			_rplayClient.play(itemIdx, _playlistId)
	}

	private fun askPauseResume() {
		_rplayClient.pause()
	}

	private fun askNext() {
		if (_playlistId != 0L) {
			val idx = (_mediaIdx+1) % _playlistItems.size
			_rplayClient.play(idx, _playlistId)
		}
	}

	private fun askPrevious() {
		if (_playlistId != 0L) {
			var idx = _mediaIdx-1
			if (idx < 0)
				idx = _playlistItems.size-1
			_rplayClient.play(idx, _playlistId)
		}
	}

	private fun askShuffle() {
		_shuffled = !_shuffled
		_rplayClient.shuffle(_shuffled)
	}

	// if play_progress not received for 15s, stop playback
	private fun createPlaybackStoppedTask(): TimerTask {
		return object : TimerTask() {
			override fun run() {
				requireActivity().runOnUiThread {
					if (_lastPlayProgressStamp == 0L || !_isPlaying)  // wait for play_progress
						return@runOnUiThread

					val now = System.currentTimeMillis()
					val isStopped = ((now - _lastPlayProgressStamp) > 15 * 1000)

					if (isStopped) {
						_isPlaying = false
						onPlaybackStop()
					}
				}
			}
		}
	}

	// helpers
	private fun currentMedia(): String {
		return if (_mediaIdx == -1 || _playlistItems.isEmpty())
			""
		else
			_playlistItems[_mediaIdx].id
	}

	private fun toPlaylistItem(item: String): PlaylistItem {
		val p = File(item)

		val fileName = p.nameWithoutExtension
		var title = fileName
		var artist = ""

		val match = findMatch(fileName)
		if (match != null) {
			title = match.groups[1]!!.value
			artist = match.groups[2]!!.value
		}
		else {  // at least remove yb hash for yb media
			val match = YOUTUBE_PATTERN.matchEntire(fileName)
			if (match != null)
				title = match.groups[1]!!.value
		}

		return PlaylistItem(title, artist, item)
	}

	private fun findMatch(fileName: String) : MatchResult? {

		var match: MatchResult? = CERCLE_PATTERN.matchEntire(fileName)
		if (match != null)
			return match

		match = BE_AT_TV_PATTERN.matchEntire(fileName)
		if (match != null)
			return match

		match = LIVE_AT_PATTERN.matchEntire(fileName)
		if (match != null)
			return match

		return null
	}

	private fun dummyContent(view: View) {

		view.current_title.text = "Boris Brejcha @ Tommorowland 2018"
		view.current_artist.text = "Boris Brejcha"

		val position = (32*69 + 46) * 1000000000L
		val duration = (59*60 + 25) * 1000000000L
		view.time.text = formatDuration(position / 1000000000L)
		view.length.text = formatDuration(duration / 1000000000L)
		view.timeline.progress = (position.toDouble() / duration.toDouble() * 100.0).toInt()

		view.playlist_items.adapter = PlaylistAdapter(requireContext(), Dummy.playlistContent())
	}

	// utils
	private fun formatDuration(seconds: Long): String {
		return DateUtils.formatElapsedTime(seconds)
	}

	// fragment_player items
	private var _playlistId = 0L
	private var _playlistItems = mutableListOf<PlaylistItem>()
	private var _mediaIdx = -1
	private var _position = 0L  // in ns
	private var _duration = 0L  // in ns
	private var _lastPlayProgressStamp = 0L  // in ms
	private var _isPlaying = false
	private var _shuffled = false

	private val _scheduler = Timer()

	private lateinit var _rplayClient: RemotePlayerClient

	private val CERCLE_PATTERN = Regex("(.*?) @ (.*) (?:for|on) Cercle-(.{11})")
	private val BE_AT_TV_PATTERN = Regex("BE-AT.TV: (.*?) [@-] (.*) \\(BE-AT.TV\\)-(.{11})")
	private val LIVE_AT_PATTERN = Regex("(.*?) - Live @ (.*)-(\\S{11})")
	private val YOUTUBE_PATTERN = Regex("(.+)-\\S{11}")
}