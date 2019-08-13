package remoteplayer.arplay

/* store patches in a filesystem like structure
\note for large number of patches list based implementation can be slow (due to find) */
class ViewFSNode(val label: String) {

	fun addPath(path: String) {

		var parent: ViewFSNode = this

		for (pathElem in path.split("/")) {
			if (pathElem.isEmpty())
				continue  // ignore first ""

			val node = parent._children.find { it.label == pathElem }
			if (node == null) {
				val newNode = ViewFSNode(pathElem)
				parent._children.add(newNode)
				parent = newNode
			}
			else
				parent = node
		}
	}

	fun getPath(path: String): ViewFSNode? {

		var parent: ViewFSNode = this

		for (pathElem in path.split("/")) {
			if (pathElem.isEmpty())
				continue  // ignore first ""

			val node = parent._children.find { it.label == pathElem }
			if (node != null)
				parent = node
			else
				return null  // not found
		}

		return parent
	}

	fun size(): Int {
		return _children.size
	}

	fun get(i: Int): ViewFSNode {
		return _children[i]
	}

	fun list(): List<String> {
		val result = mutableListOf<String>()
		for (node in _children)
			result.add(node.label)
		return result
	}

	private val _children: MutableList<ViewFSNode> = mutableListOf()
}
