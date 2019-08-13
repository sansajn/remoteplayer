package remoteplayer.arplay

import android.content.Context
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.BaseAdapter
import android.widget.TextView

class LibraryAdapter(val context: Context, val items: List<String>) : BaseAdapter() {

	override fun getView(position: Int, convertView: View?, parent: ViewGroup?): View {
		val view: View
		val holder: ViewHolder

		if (convertView != null) {
			view = convertView
			holder = view.tag as ViewHolder
		}
		else {
			view = LayoutInflater.from(context).inflate(R.layout.library_item, parent, false)
			holder = ViewHolder(view)
			view.tag = holder
		}

		val item = items[position]
		holder.title.text = item

		return view
	}

	override fun getItem(position: Int): Any {
		return items[position]
	}

	override fun getItemId(position: Int): Long {
		return position.toLong()
	}

	override fun getCount(): Int {
		return items.size
	}

	private class ViewHolder {  // reduces number of findViewById calls

		constructor(view: View?) {
			title = view?.findViewById(R.id.title) as TextView
		}

		val title: TextView
	}
}