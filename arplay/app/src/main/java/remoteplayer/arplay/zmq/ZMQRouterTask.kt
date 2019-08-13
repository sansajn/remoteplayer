package remoteplayer.arplay.zmq

import android.os.AsyncTask
import org.zeromq.ZMQ

/*! \note needs to be run on UI thread
\code
runOnUiThread { ZMQRouterTask(this@MainActivity::answerQuestion).execute(_router) }
\endcode */
class ZMQRouterTask(private val delegate: (String) -> String): AsyncTask<Any, Void, MutableList<Pair<ByteArray, String>>>() {

	// params (routerSocket: ZMQ.Socket)
	override fun doInBackground(vararg params: Any?): MutableList<Pair<ByteArray, String>> {

		val router = params[0] as ZMQ.Socket
		_router = router
		val questions = mutableListOf<Pair<ByteArray, String>>()

		val id: ByteArray? = router.recv(ZMQ.NOBLOCK)
		if (id != null) {  // we have a question there
			val question = router.recvStr()
			questions.add(Pair(id, question))
		}

		return questions
	}

	override fun onPostExecute(questions: MutableList<Pair<ByteArray, String>>) {
		super.onPostExecute(questions)

		for ((id, question) in questions) {
			val answer = delegate(question)
			replay(id, answer)
		}
	}

	private fun replay(id: ByteArray, answer: String) {
		_router.sendMore(id)
		_router.send(answer)
	}

	private lateinit var _router: ZMQ.Socket
}
