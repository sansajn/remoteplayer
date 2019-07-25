package remoteplayer.arplay.zmq

import android.os.AsyncTask
import org.zeromq.ZMQ

class ZMQSubscriberTask(private val delegate: (String) -> Unit) : AsyncTask<Any, Void, MutableList<String>>() {

	override fun doInBackground(vararg params: Any): MutableList<String> {
		val sub = params[0] as ZMQ.Socket
		val commands = mutableListOf<String>()

		while (true) {
			val d = sub.recvStr(ZMQ.NOBLOCK)
			if (d != null)
				commands.add(d)
			else
				break
		}

		return commands
	}

	override fun onPostExecute(commands: MutableList<String>) {
		super.onPostExecute(commands)

		if (commands.isNotEmpty())
			for (cmd in commands)
				delegate(cmd)
	}
}
