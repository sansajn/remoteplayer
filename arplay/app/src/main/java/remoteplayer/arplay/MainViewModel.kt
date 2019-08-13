package remoteplayer.arplay

import android.arch.lifecycle.LiveData
import android.arch.lifecycle.MutableLiveData
import android.arch.lifecycle.ViewModel

class MainViewModel : ViewModel(), LibraryListener {

	private val _rplay = RemotePlayerClient()

	init {
		_rplay.registerListener(this)
	}

	fun remotePlayerClient(): RemotePlayerClient {
		return _rplay
	}

	fun libraryContent(): LiveData<List<String>> {
		return _library
	}

	override fun mediaLibrary(items: List<String>) {
		_library.value = items
	}

	private val _library = MutableLiveData<List<String>>()
}
