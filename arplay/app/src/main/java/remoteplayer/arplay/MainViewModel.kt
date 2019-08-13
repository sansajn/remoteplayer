package remoteplayer.arplay

import android.arch.lifecycle.LiveData
import android.arch.lifecycle.MutableLiveData
import android.arch.lifecycle.ViewModel

class MainViewModel : ViewModel() {

	fun libraryContent(): LiveData<List<String>> {
		return _library
	}

	fun updateLibraryContent(library: List<String>) {
		_library.value = library
	}

	private val _library = MutableLiveData<List<String>>()
}
