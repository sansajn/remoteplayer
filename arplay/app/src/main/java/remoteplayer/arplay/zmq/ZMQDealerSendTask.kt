package remoteplayer.arplay.zmq

import android.os.AsyncTask
import org.zeromq.ZMQ

// router <- dealer connection dealer ask part task implementation
class ZMQDealerSendTask() : AsyncTask<Any, Void, Unit>() {

	// (dealerSocket: ZMQ.Socket, question: String)
	override fun doInBackground(vararg params: Any?) {
		val dealer = params[0] as ZMQ.Socket
		val question = params[1] as List<String>
		question.forEach { dealer.send(it) }
	}
}
