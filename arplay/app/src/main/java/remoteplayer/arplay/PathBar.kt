package remoteplayer.arplay

import android.content.Context
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Toast
import kotlinx.android.synthetic.main.pathbar_item.view.*

class PathBarListAdapter(var c: Context, var list: List<String>, val changedDirectoryHadler: (String) -> Unit)
	: RecyclerView.Adapter<RecyclerView.ViewHolder>() {

	override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
		var v = LayoutInflater.from(c).inflate(R.layout.pathbar_item, parent, false)
		return Item(v)
	}

	override fun getItemCount(): Int {
		return list.size
	}

	override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
		(holder as Item).bindData(list[position], itemPath(position), changedDirectoryHadler)
	}

	private fun itemPath(position: Int): String {
		var path = "/"
		for (i in 0 until position)
			path += list[i] + '/'
		path += list[position]

		return path
	}

	private class Item(itemView: View) : RecyclerView.ViewHolder(itemView) {
		fun bindData(item: String, path: String, handler: (String) -> Unit) {
			itemView.textView.text = item
			itemView.setOnClickListener {
				handler(path)
				Toast.makeText(itemView.context, "$path", Toast.LENGTH_SHORT).show() }
		}
	}
}
