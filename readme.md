# remoteplayer

> TODO: describe what remoteplayer is and how cen be used ...


## How to install ?

### Server part (rplay)

- compile with `$ scons -j11` command
- run remoteplayer with `$ ./rplay /path/to/media/library` command

That is it, remoteplayer (rplay) is now listenning on `tcp://*:5557` address.


### Client part (grplay)

Run grplay (graphics remote player client) with rplay IP address and port number

```
$ ./grplay 192.168.0.111 3333
```

or run rplayc (console remote player client) with rplay IP address, like

```
$ ./rplayc 192.168.0.111
```


## How to configure ?

### Server part (rplay)

Remote Player Server (rplay) can be configured with `rplay.conf` config file stored in remoteplayer directory, see sample

```js
{
	"rplay":{
		"media_home":"/home/me/Music",
		"port":13333,
		"log_file":"~/rplay.log"
	}
}
```

where

**rplay.media_home**: specify media library directory

**rplay.port**: specify ZMQ interface port number

**rplay.log_file**: will redirect logging into specified file


### rsync

> describe rsinc configuration I use ...


