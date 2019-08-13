package remoteplayer.arplay

import android.arch.lifecycle.Observer
import android.arch.lifecycle.ViewModelProviders
import android.os.Bundle
import android.support.v4.app.Fragment
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.AdapterView
import android.widget.Toast
import kotlinx.android.synthetic.main.fragment_library.*


class LibraryFragment : Fragment() {

	override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
		return inflater.inflate(R.layout.fragment_library, container, false)
	}

	override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
		super.onViewCreated(view, savedInstanceState)

		val viewModel = ViewModelProviders.of(requireActivity()).get(MainViewModel::class.java)
		viewModel.libraryContent().observe(this, Observer { items ->
			populateLibraryList(items!!)
		})
	}

	private fun populateLibraryList(items: List<String>) {

		val root = ViewFSNode("/")
		items.forEach { root.addPath(it) }

		_path = findFirstDirectoryWithContent(root)

		val music = root.getPath(_path)
		if (music != null) {
			_pathContent = music.list()
			library_list.adapter = LibraryAdapter(requireContext(), _pathContent)
		}

		library_list.onItemClickListener = AdapterView.OnItemClickListener { parent, view, position, id ->

			if (_pathContent != null) {
				val item = "$_path/${_pathContent[position]}"
				Log.d("app", "item '$item' ($position) clicked")
				Toast.makeText(requireContext(), "item '$item' selected", Toast.LENGTH_LONG).show()
			}
			else
				Toast.makeText(requireContext(), "error: unknown item selected", Toast.LENGTH_LONG).show()
		}

		library_path.text = _path
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

	private var _path: String = ""
	private var _pathContent = listOf<String>()
}
