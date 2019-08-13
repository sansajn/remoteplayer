package remoteplayer.arplay.zmq

import android.os.AsyncTask
import org.zeromq.ZMQ

// router <- dealer connection dealer task implementation
class ZMQDealerRecvTask(private val delegate: (String) -> Unit) : AsyncTask<Any, Void, MutableList<String>>() {

	// (dealerSocket: ZMQ.Socket)
	override fun doInBackground(vararg params: Any?): MutableList<String> {

		val dealer = params[0] as ZMQ.Socket
		val answers = mutableListOf<String>()

		while (true) {
			val answer = dealer.recvStr(ZMQ.NOBLOCK)
			if (answer != null)
				answers.add(answer)
			else
				break
		}

		return answers
	}

	override fun onPostExecute(answers: MutableList<String>) {
		super.onPostExecute(answers)

		for (answer in answers)
			delegate(answer)
	}
}
