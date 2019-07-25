package remoteplayer.arplay

import android.os.Bundle
import android.support.design.widget.NavigationView
import android.support.v4.view.GravityCompat
import android.support.v7.app.ActionBarDrawerToggle
import android.support.v7.app.AppCompatActivity;
import android.support.v7.preference.PreferenceManager
import android.view.Menu
import android.view.MenuItem
import android.widget.TextView
import android.widget.Toast

import kotlinx.android.synthetic.main.activity_main.*
import kotlinx.android.synthetic.main.app_bar_main.*

class MainActivity : AppCompatActivity(), NavigationView.OnNavigationItemSelectedListener {

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_main)
		setSupportActionBar(toolbar)

		val toggle = ActionBarDrawerToggle(this, drawer_layout, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close)
		drawer_layout.addDrawerListener(toggle)
		toggle.syncState()

		nav_view.setNavigationItemSelectedListener(this)

		val transaction = supportFragmentManager.beginTransaction()
		transaction.add(R.id.fragment_container, createPlayerFragment())
		transaction.commit()

		val headerView = nav_view.getHeaderView(0)
		val navHeaderVersion = headerView.findViewById(R.id.navHeaderVersion) as TextView
		navHeaderVersion.text = BuildConfig.GitVersion
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

	override fun onNavigationItemSelected(item: MenuItem): Boolean {
		val transaction = supportFragmentManager.beginTransaction()

		// Handle navigation view item clicks here.
		when (item.itemId) {
			R.id.nav_player -> {
				transaction.replace(R.id.fragment_container, createPlayerFragment())
			}
			R.id.nav_settings -> {
				transaction.replace(R.id.fragment_container, SettingsFragment())
			}
		}

		transaction.commit()

		drawer_layout.closeDrawer(GravityCompat.START)
		return true
	}

	private fun createPlayerFragment(): PlayerFragment {

		if (_rplay == null) {
			val sharedPref = PreferenceManager.getDefaultSharedPreferences(this)
			val mediaServerAddress = sharedPref.getString(getString(R.string.pref_key_server_address), "")

			_rplay = RemotePlayerClient(this)
			_rplay?.connect("tcp://$mediaServerAddress", 23333)

			Toast.makeText(this, "connecting to tcp://$mediaServerAddress:23333", Toast.LENGTH_LONG)
		}

		val frag = PlayerFragment()
		frag.setup(_rplay!!)
		return frag
	}

	private var _rplay: RemotePlayerClient? = null
}
