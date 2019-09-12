package remoteplayer.arplay

import android.arch.lifecycle.ViewModelProviders
import android.os.Bundle
import android.support.v4.app.Fragment
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.support.v7.widget.helper.ItemTouchHelper
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

		_player = Player(_rplayClient)

		enablePlaylistDragAndDrop(view.playlist_items)

		view.playlist_items.layoutManager = LinearLayoutManager(requireContext())
		view.playlist_items.hasFixedSize()
		view.playlist_items.adapter = PlaylistRecyclerAdapter(requireContext(), _player)
//		dummyContent(view)

		setupPlaybackControls(view)

		_scheduler.schedule(createPlaybackStoppedTask(), 100, 500)
	}

	override fun onDestroyView() {
		super.onDestroyView()
		_scheduler.cancel()
		_rplayClient.removeListener(this)
	}

	override fun playlistContent(id: Long, items: List<String>) {
		val playlistItems = mutableListOf<PlaylistItem>()
		for (item in items)
			playlistItems.add(toPlaylistItem(item))

		_player.updatePlaylist(id, playlistItems)

		playlist_items.adapter = PlaylistRecyclerAdapter(requireContext(), _player)
	}

	override fun playProgress(position: Long, duration: Long, playlistId: Long, mediaIdx: Long, playbackState: Int, mode: Int) {

		if (playlistId != _player.playlistId())
			return  // playlist not match, wait for playlist_content message first

		_position = position
		_duration = duration

		time.text = formatDuration(position / 1000000000)
		length.text = formatDuration(duration / 1000000000)
		timeline.progress = (position.toDouble() / duration.toDouble() * 100.0).toInt()

		if (_player.currentMediaIdx() != mediaIdx.toInt()) {
			_player.updateMediaIdx(mediaIdx.toInt())
			val currentMedia = _player.currentMedia()
			current_title.text = currentMedia.title
			current_artist.text = currentMedia.artist
		}

		if (!_isPlaying && playbackState == 1) {  // playing
			_isPlaying = true
			onPlaybackPlay()
		}
		else if (!_isPlaying && playbackState == 2) {  // paused
			_isPlaying = true
			onPlaybackStop()
		}

		_player.updateShuffled(mode == 1)

		_lastPlayProgressStamp = System.currentTimeMillis()
	}

	// events
	private fun onPlaybackStop() {
		play_pause.setImageResource(R.drawable.ic_baseline_play_arrow_24px)
		play_pause.setOnClickListener {
			if (_isPlaying)
				_player.pauseResume()
			else
				_player.play()
			onPlaybackPlay()
		}
	}

	private fun onPlaybackPlay() {
		play_pause.setImageResource(R.drawable.ic_baseline_pause_24px)
		play_pause.setOnClickListener {
			_player.pauseResume()
			_isPlaying = false
			onPlaybackStop()
		}
	}

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

	// setup UI
	private fun setupPlaybackControls(view: View) {
		view.shuffle.setOnClickListener { _player.shuffle() }
		view.previous.setOnClickListener { _player.previous() }
		view.next.setOnClickListener { _player.next() }

		view.timeline.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
			override fun onStopTrackingTouch(seekBar: SeekBar?) {
				if (seekBar != null) {
					val seconds = ((seekBar.progress).toDouble() / 100.0 * _duration.toDouble() / 1000000000.0).toInt()
					_player.seek(seconds)
				}
			}

			override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {}
			override fun onStartTrackingTouch(seekBar: SeekBar?) {}
		})
	}

	private fun enablePlaylistDragAndDrop(playlistView: RecyclerView) {

		val simpleItemTouchCallback = object : ItemTouchHelper.SimpleCallback(ItemTouchHelper.UP or ItemTouchHelper.DOWN or ItemTouchHelper.START or ItemTouchHelper.END, 0) {

			override fun onMove(recycler: RecyclerView, holder: RecyclerView.ViewHolder, target: RecyclerView.ViewHolder): Boolean {
				_from = holder.adapterPosition
				_to = target.adapterPosition
				return true
			}

			override fun onSwiped(holder: RecyclerView.ViewHolder, direction: Int) {
				// do nothing ...
			}

			override fun onSelectedChanged(viewHolder: RecyclerView.ViewHolder?, actionState: Int) {
				super.onSelectedChanged(viewHolder, actionState)
				if (actionState == ItemTouchHelper.ACTION_STATE_DRAG)
					viewHolder?.itemView?.alpha = 0.5f
			}

			override fun clearView(recyclerView: RecyclerView, viewHolder: RecyclerView.ViewHolder) {
				super.clearView(recyclerView, viewHolder)
				viewHolder?.itemView?.alpha = 1.0f
				val adapter = recyclerView.adapter as PlaylistRecyclerAdapter
				adapter.moveItem(_from, _to)
				adapter.notifyItemMoved(_from, _to)
			}

			private var _from = 0
			private var _to = 0
		}

		_playlistItemTouchHelper = ItemTouchHelper(simpleItemTouchCallback)
		_playlistItemTouchHelper.attachToRecyclerView(playlistView)
	}

	override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
		return inflater.inflate(R.layout.fragment_player, container, false)
	}

	// helpers
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

//	private fun dummyContent(view: View) {
//
//		view.current_title.text = "Boris Brejcha @ Tommorowland 2018"
//		view.current_artist.text = "Boris Brejcha"
//
//		val position = (32*69 + 46) * 1000000000L
//		val duration = (59*60 + 25) * 1000000000L
//		view.time.text = formatDuration(position / 1000000000L)
//		view.length.text = formatDuration(duration / 1000000000L)
//		view.timeline.progress = (position.toDouble() / duration.toDouble() * 100.0).toInt()
//
//		view.playlist_items.adapter = PlaylistAdapter(requireContext(), Dummy.playlistContent())
//	}

	// utils
	private fun formatDuration(seconds: Long): String {
		return DateUtils.formatElapsedTime(seconds)
	}

	// fragment_player items
	private var _position = 0L  // in ns
	private var _duration = 0L  // in ns
	private var _lastPlayProgressStamp = 0L  // in ms
	private var _isPlaying = false

	private val _scheduler = Timer()

	private lateinit var _rplayClient: RemotePlayerClient
	private lateinit var _player: Player
	private lateinit var _playlistItemTouchHelper: ItemTouchHelper

	private val CERCLE_PATTERN = Regex("(.*?) @ (.*) (?:for|on) Cercle-(.{11})")
	private val BE_AT_TV_PATTERN = Regex("BE-AT.TV: (.*?) [@-] (.*) \\(BE-AT.TV\\)-(.{11})")
	private val LIVE_AT_PATTERN = Regex("(.*?) - Live @ (.*)-(\\S{11})")
	private val YOUTUBE_PATTERN = Regex("(.+)-\\S{11}")
}
