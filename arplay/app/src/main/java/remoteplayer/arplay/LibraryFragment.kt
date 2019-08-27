package remoteplayer.arplay

import android.arch.lifecycle.Observer
import android.arch.lifecycle.ViewModelProviders
import android.os.Bundle
import android.support.v4.app.Fragment
import android.support.v7.widget.LinearLayoutManager
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.Toast
import kotlinx.android.synthetic.main.fragment_library.*
import remoteplayer.arplay.library.DirectorySort
import remoteplayer.arplay.library.isDirectory


class LibraryFragment : Fragment() {

	override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
		super.onViewCreated(view, savedInstanceState)

		val viewModel = ViewModelProviders.of(requireActivity()).get(MainViewModel::class.java)
		_rplay = viewModel.remotePlayerClient()
		viewModel.libraryContent().observe(this, Observer { items ->
			populateLibraryList(items!!)
		})

		// pathbar
		path_bar.layoutManager = LinearLayoutManager(requireContext(), RecyclerView.HORIZONTAL, false)
		path_bar.hasFixedSize()
		path_bar.adapter = PathBarListAdapter(requireContext(), listOf("home", "ja", "Music", "2019"), this::changeLibraryDirectory)
	}

	private fun changeLibraryDirectory(path: String) {
		val musicFiles = _root.getPath(path)
		if (musicFiles != null) {
			_path = path
			_pathContent = musicFiles.list().sortedWith(DirectorySort)
			library_list.adapter = LibraryAdapter(requireContext(), _pathContent)
			val pathList = _path.split('/')
			path_bar.adapter = PathBarListAdapter(requireContext(), pathList.subList(1, pathList.size), this::changeLibraryDirectory)
		}
		else
			Toast.makeText(requireContext(), "directory $path is empty", Toast.LENGTH_LONG).show()
	}

	private fun populateLibraryList(items: List<String>) {

		_root.clear()
		items.forEach { _root.addPath(it) }

		val path = findFirstDirectoryWithContent(_root)
		changeLibraryDirectory(path)

		library_list.onItemClickListener = AdapterView.OnItemClickListener { parent, view, position, id ->

			if (_pathContent != null) {
				val item = "$_path/${_pathContent[position]}"
				if (isDirectory(item))
					changeLibraryDirectory(item)
				else
					_rplay.addToPlaylist(listOf(item))

				Toast.makeText(requireContext(), "item '$item' selected", Toast.LENGTH_LONG).show()
			}
			else
				Toast.makeText(requireContext(), "error: unknown item selected", Toast.LENGTH_LONG).show()
		}
	}

	private fun findFirstDirectoryWithContent(fs: ViewFSNode): String {

		var node = fs
		var path = ""
		while (node.size() == 1) {
			node = node.get(0)
			path += "/" + node.label
		}

		return path
	}

	override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
		return inflater.inflate(R.layout.fragment_library, container, false)
	}

	private var _path: String = ""
	private var _pathContent = listOf<String>()
	private lateinit var _rplay: RemotePlayerClient
	private var _root = ViewFSNode("/")  // virtual file system
}
