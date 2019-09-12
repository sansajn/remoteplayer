package remoteplayer.arplay

import android.content.Context
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.BaseAdapter
import android.widget.TextView
import android.widget.Toast
import kotlinx.android.synthetic.main.simple_playlist_item.view.*

data class PlaylistItem(
	val title: String,
	val artist: String,
	val id: String  // item path
)

class PlaylistListAdapter(val context: Context, val items: List<PlaylistItem>) : BaseAdapter() {

	override fun getView(position: Int, convertView: View?, parent: ViewGroup?): View {
		val view: View
		val holder: ViewHolder

		if (convertView != null) {
			view = convertView
			holder = view.tag as ViewHolder
		}
		else {
			view = LayoutInflater.from(context).inflate(R.layout.simple_playlist_item, parent, false)
			holder = ViewHolder(view)
			view.tag = holder
		}

		val item = items[position]
		holder.title.text = item.title
		holder.subtitle.text = item.artist

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
			subtitle = view?.findViewById(R.id.subtitle) as TextView
		}

		val title: TextView
		val subtitle: TextView
	}
}

class PlaylistRecyclerAdapter(var c: Context, private val _player: Player) : RecyclerView.Adapter<RecyclerView.ViewHolder>() {

	override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): RecyclerView.ViewHolder {
		var v = LayoutInflater.from(c).inflate(R.layout.simple_playlist_item, parent, false)
		return Item(v, _player)
	}

	override fun getItemCount(): Int {
		return _player.playlist().size
	}

	override fun onBindViewHolder(holder: RecyclerView.ViewHolder, position: Int) {
		(holder as Item).bindData(_player.playlist()[position], position)
	}

	class Item(itemView: View, private val _player: Player) : RecyclerView.ViewHolder(itemView) {
		fun bindData(item: PlaylistItem, idx: Int) {
			itemView.title.text = item.title
			itemView.subtitle.text = item.artist
			itemView.setOnClickListener { _player.play(idx) }
		}
	}
}
