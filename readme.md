# How to install remoteplayer

## Server part

- compile with '$ scons -j8' command
- run remoteplayer with '$ ./rplay /path/to/media/library' command

That is it, remoteplayer (rplay) is now listenning on `tcp://*:5557` address.


## Client part

Run grplay (graphics remote player client) with rplay IP address and port number

```
$ ./grplay 192.168.0.111 3333
```

or run rplayc (console remote player client) with rplay IP address, like

```
$ ./rplayc 192.168.0.111
```

# Configure remoteplayer

Remote Player Server (rplay) can be configured with

[Sample: rplay.conf
```js
{
	"rplay":{
		"media_home":"/home/me/Music",
		"port":13333,
		"log_file":"~/rplay.log"
	}
}
```

--- end of sample] JSON like file stored as `rplay.conf` in remoteplayer directory where

**rplay.media_home**: specify media library directory

**rplay.port**: specify ZMQ interface port number

**rplay.log_file**: will redirect logging into specified file
