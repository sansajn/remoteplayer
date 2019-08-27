package remoteplayer.arplay

import android.arch.lifecycle.ViewModelProviders
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
import remoteplayer.arplay.library.LibraryFragment

class MainActivity : AppCompatActivity(), NavigationView.OnNavigationItemSelectedListener {

	override fun onCreate(savedInstanceState: Bundle?) {
		super.onCreate(savedInstanceState)
		setContentView(R.layout.activity_main)
		setSupportActionBar(toolbar)

		val toggle = ActionBarDrawerToggle(this, drawer_layout, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close)
		drawer_layout.addDrawerListener(toggle)
		toggle.syncState()

		nav_view.setNavigationItemSelectedListener(this)  // TODO: rename nav_view to something meaningful

		_viewModel = ViewModelProviders.of(this).get(MainViewModel::class.java)

		connectPlayer()
		askLibraryContent()

		loadPlayerUI()
//		loadLibraryUI()

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
		when (item.itemId) {
			R.id.action_settings -> {
				loadSettingsUI()
				return true
			}

			else -> return super.onOptionsItemSelected(item)
		}
	}

	override fun onNavigationItemSelected(item: MenuItem): Boolean {

		// Handle navigation view item clicks here.
		when (item.itemId) {
			R.id.nav_player -> loadPlayerUI()
			R.id.nav_library -> loadLibraryUI()
			R.id.nav_settings -> loadSettingsUI()
		}

		drawer_layout.closeDrawer(GravityCompat.START)
		return true
	}

	private fun loadPlayerUI() {
		supportFragmentManager.beginTransaction()
			.replace(R.id.fragment_container, PlayerFragment())
			.commit()
	}

	private fun loadLibraryUI() {
		supportFragmentManager.beginTransaction()
			.replace(R.id.fragment_container, LibraryFragment())
			.commit()
	}

	private fun loadSettingsUI() {
		supportFragmentManager.beginTransaction()
			.replace(R.id.fragment_container, SettingsFragment())
			.commit()
	}

	private fun connectPlayer() {
		val sharedPref = PreferenceManager.getDefaultSharedPreferences(this)
		val mediaServerAddress = sharedPref.getString(getString(R.string.pref_key_server_address), "")
		_viewModel.remotePlayerClient().connect("tcp://$mediaServerAddress", 23333)

		Toast.makeText(this, "connecting to tcp://$mediaServerAddress:23333", Toast.LENGTH_LONG).show()
	}

	private fun askLibraryContent() {
		val libraryContent = _viewModel.libraryContent().value
		if (libraryContent == null)
			_viewModel.remotePlayerClient().listMedia()
		else if (libraryContent.isEmpty())
			_viewModel.remotePlayerClient().listMedia()
	}

	lateinit var _viewModel: MainViewModel
}
