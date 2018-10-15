package com.remoteplayer.arplay

import android.os.Bundle
import android.support.design.widget.Snackbar
import android.support.v7.app.AppCompatActivity;
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.widget.ArrayAdapter

import kotlinx.android.synthetic.main.activity_player.*
import kotlinx.android.synthetic.main.content_player.*


fun formatDuration(dur: Int): String {
	var min = dur/60
	var sec = dur % 60
	return "$min:$sec"
}

class PlayerActivity : AppCompatActivity() {

	var rplay: DummyRPlayClient = DummyRPlayClient()

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_player)
		setSupportActionBar(toolbar)

		fab.setOnClickListener { view ->
			Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
					  .setAction("Action", null).show()
		}

		media_text_view.text = rplay.media()
		position_text_view.text = formatDuration(rplay.position())
		durationTextView.text = formatDuration(rplay.duration())
		mediaSeekBar.max = rplay.duration()
		mediaSeekBar.progress = rplay.position()

		// setup control buttons handlers
		prev_button.setOnClickListener {
			Log.d("arplay", "prev button clicked")
		}

		play_pause_button.setOnClickListener {
			Log.d("arplay", "play/pause buton clicked")
			rplay.play(0, 1)
		}

		stop_button.setOnClickListener {
			Log.d("arplay", "stop button clicked")
			rplay.stop()
		}

		next_button.setOnClickListener {
			Log.d("arplay", "next button clicked")
		}

		// fill playlist
		val adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, rplay.playlist())
		playlist_list_view.adapter = adapter

		// Example of a call to a native method
		val textString: String = stringFromJNI()
	}

	override fun onCreateOptionsMenu(menu: Menu): Boolean {
		// Inflate the menu; this adds items to the action bar if it is present.
		menuInflater.inflate(R.menu.menu_player, menu)
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

	/**
	 * A native method that is implemented by the 'native-lib' native library,
	 * which is packaged with this application.
	 */
	external fun stringFromJNI(): String

	companion object {

		// Used to load the 'native-lib' library on application startup.
		init {
			System.loadLibrary("native-lib")
		}
	}
}
