package com.remoteplayer.arplay

class DummyRPlayClient {

	// observers
	external fun media(): String
	external fun position(): Int
	external fun duration(): Int
	external fun playlist(): List<String>
	external fun library(): List<String>

	// commands
	external fun play(playlistId: Long, playlistIdx: Long)
	external fun pause()
	external fun stop()
	external fun seek(position: Long, media: String)
	external fun prev()
	external fun next()
	external fun volume(value: Int)
	external fun playlistAdd(items: List<String>)
	external fun playlistRemove(playlistId: Long, playlistIdx: Long)
	external fun askIdentity()
	external fun askLibrary()
	external fun close()
}