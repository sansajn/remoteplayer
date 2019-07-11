package remoteplayer.arplay

import android.os.Bundle
import android.support.design.widget.Snackbar
import android.support.v7.app.AppCompatActivity;
import android.text.format.DateUtils
import android.view.Menu
import android.view.MenuItem
import android.widget.AdapterView
import android.widget.SeekBar
import android.widget.Toast

import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.player.*
import java.io.File
import java.util.*

class MainActivity : AppCompatActivity(), RemotePlayerListener {

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_main)
		setSupportActionBar(toolbar)

		fab.setOnClickListener { view ->
			Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
				.setAction("Action", null).show()
		}

		val items = listOf(
			PlaylistItem("Kollektiv Turmstrasse @ Tomorrowland 2018", "Kollektiv Turmstrasse", "Kollektiv Turmstrasse @ Tomorrowland 2018.opus"),
			PlaylistItem("Artbat @ Bondinho Pao Acucar for Cercle", "Artbat", "Artbat @ Bondinho Pao Acucar for Cercle.opus"),
			PlaylistItem("Adam Bayer @ Ultra 2019", "Adam Bayer", "Adam Bayer @ Ultra 2019.opus"),
			PlaylistItem("Jeremy Olander @ Cedergrenska Tornet for Cercle", "Jeremy Olander", "Jeremy Olander @ Cedergrenska Tornet for Cercle.opus"),
			PlaylistItem("Boris Brejcha @ Tommorowland 2018", "Boris Brejcha", "Boris Brejcha @ Tommorowland 2018.opus"),
			PlaylistItem("Monika Kruse @ Montparnasse Tower Observation Deck for Cercle", "Monika Kruse", "Monika Kruse @ Montparnasse Tower Observation Deck for Cercle.opus"),
			PlaylistItem("Tale Of Us @ Paris Charles de Gaulle Airport for Cercle", "Tale Of Us", "Tale Of Us @ Paris Charles de Gaulle Airport for Cercle.opus"),
			PlaylistItem("ANNA @ Rave On Snow 2017", "ANNA", "ANNA @ Rave On Snow 2017.opus"),
			PlaylistItem("Adriatique @ Diynamic 2018", "Adriatique", "Adriatique @ Diynamic 2018.opus"),
			PlaylistItem("Amelie Lens @ LaPlage de Glazart for Cercle", "Amelie Lens", "Amelie Lens @ LaPlage de Glazart for Cercle.opus"),
			PlaylistItem("Andy Bros @ Diynamic 2018", "Andy Bros", "Andy Bros @ Diynamic 2018.opus"),
			PlaylistItem("Charlotte de Witte @ Awakenings ADE 2018", "Charlotte de Witte", "Charlotte de Witte @ Awakenings ADE 2018.opus"))

		playlist_items.adapter = PlaylistAdapter(this, items)

		playlist_items.onItemClickListener = AdapterView.OnItemClickListener { parent, view, position, id ->
			askPlay(position)
		}

		timeline.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {

			override fun onStopTrackingTouch(seekBar: SeekBar?) {

				if (seekBar != null) {
					val media = currentMedia()
					if (media.isNotEmpty()) {
						val seconds = ((seekBar.progress).toDouble() / 100.0 * _duration.toDouble() / 1000000000.0).toInt()
						_rplayClient.seek(seconds, media)
					}
					else {
						Toast.makeText(this@MainActivity, "unknown played item, seek ignored", Toast.LENGTH_SHORT).show()
					}
				}
			}

			override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {}
			override fun onStartTrackingTouch(seekBar: SeekBar?) {}
		})

		_rplayClient.connect("tcp://192.168.0.108", 23333)
		_rplayClient.clientReady()


		val checkPlaybackStoppedTask = object : TimerTask() {
			override fun run() {
				runOnUiThread {
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

		_scheduler.schedule(checkPlaybackStoppedTask, 100, 500)
	}

	fun askPlay(itemIdx: Int) {
		if (_playlistId != 0L)
			_rplayClient.play(itemIdx, _playlistId)
	}

	fun askPause() {
		_rplayClient.pause()
	}

	fun askStop() {
		_rplayClient.stop()
	}

	fun onPlaybackStop() {
		play_pause.setImageResource(android.R.drawable.ic_media_play)
		play_pause.setOnClickListener {
/*
			if (_isPlaying && _isPaused)
				askPause()
			else
				askPlay(_mediaIdx)
*/
			askPlay(_mediaIdx)

			onPlaybackPlay()
		}
	}

	fun onPlaybackPlay() {
		play_pause.setImageResource(android.R.drawable.ic_media_pause)
		play_pause.setOnClickListener {
//			askPause()
			askStop()
			_isPlaying = false
			onPlaybackStop()
		}
	}

	fun currentMedia(): String {
		if (_mediaIdx == -1 || _playlistItems.isEmpty())
			return ""
		else
			return _playlistItems[_mediaIdx].id
	}

	fun formatDuration(seconds: Long): String {
		return DateUtils.formatElapsedTime(seconds)
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

		if (!_isPlaying) {
			_isPlaying = true
			onPlaybackPlay()
		}


/*
		val prevIsPlaying = _isPlaying
		val prevIsPaused = _isPaused

		when (playbackState) {
			1 -> {
				_isPlaying = true
				_isPaused = false
				if (!prevIsPlaying)
					onPlaybackPlay()
			}

			2 -> {
				_isPlaying = true
				_isPaused = true
				if (!prevIsPaused)
					onPlaybackStop()
			}

			3 -> {
				_isPlaying = false
				_isPaused = false
				if (prevIsPlaying)
					onPlaybackStop()
			}
		}
*/

		_lastPlayProgressStamp = System.currentTimeMillis()
	}

	fun toPlaylistItem(item: String): PlaylistItem {
		val p = File(item)
		return PlaylistItem(p.nameWithoutExtension, "", item)
	}

	override fun playlistContent(id: Long, items: List<String>) {
		_playlistId = id

		_playlistItems.clear()
		for (item in items)
			_playlistItems.add(toPlaylistItem(item))

		playlist_items.adapter = PlaylistAdapter(this, _playlistItems)
	}

	override fun onCreateOptionsMenu(menu: Menu): Boolean {
		// Inflate the menu; this adds items to the action bar if it is present.
		menuInflater.inflate(R.menu.menu_main, menu)
		return true
	}

	override fun onOptionsItemSelected(item: MenuItem): Boolean {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		return when (item.itemId) {
			R.id.action_settings -> true
			else -> super.onOptionsItemSelected(item)
		}
	}

	// player items
	var _playlistId = 0L
	var _playlistItems = mutableListOf<PlaylistItem>()
	var _mediaIdx = -1
	var _position = 0L  // in ns
	var _duration = 0L  // in ns
	var _lastPlayProgressStamp = 0L  // in ms
	var _isPlaying = false
	var _isPaused = false

	private val _scheduler = Timer()

	private val _rplayClient = RemotePlayerClient(this, this)
}
