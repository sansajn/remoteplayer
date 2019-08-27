package remoteplayer.arplay.library

fun isDirectory(path: String): Boolean {
	return !path.contains('.')
}

fun lessCompare(a: String, b: String): Int = when {
	a < b -> -1
	a > b -> 1
	else -> 0
}

class DirectorySort {
	companion object : Comparator<String> {
		override fun compare(a: String, b: String): Int {
			val aDir = isDirectory(a)
			val bDir = isDirectory(b)

			return when {
				aDir && bDir -> lessCompare(a, b)
				aDir -> -1
				bDir -> 1
				else -> lessCompare(a, b)
			}
		}
	}
}