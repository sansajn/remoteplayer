package remoteplayer.arplay

import android.os.AsyncTask
import org.zeromq.ZMQ

class ZMQPushTask(private val _push: ZMQ.Socket, private val _messages: List<String>) : AsyncTask<Void, Void, Void?>() {

	override fun doInBackground(vararg params: Void?): Void? {
		for (m in _messages)
			_push.send(m)

		return null
	}

}
