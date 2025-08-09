# remoteplayer

Remote audio player protocol definition (see `docs/spec.txt`) with server side implementation as `rplay` and client side implementation for Android as `arplay`.

## Build

Build development docker image (`Dockerfile.dev`) by

```bash
cd rplay
make image
```

command.

With a `docker image ls` we can check `rplay-dev` docker image created

```console
$ docker image ls
REPOSITORY   TAG       IMAGE ID       CREATED         SIZE
rplay-dev    latest    8fc5f7ef6f4b   9 minutes ago   826MB
```

To build, run

```bash
make build
```

to clean, run

```bash
make clean
```

Now to run docker container and build *rplay* project run

```bash
make bash
```

Now runtime image can be build by

```bash
make runtime
```

command.

To run `rplay` within docker container run

```bash
docker run --rm -it --user $(id -u):$(id -g) -v $(pwd):/build rplay-runtime -w /build ./rplay
```

where

- `-v $(pwd):/build`: mounts your current folder (where `rplay` binary lives) as `/bin` inside the container.
- `-w /build`: seyt working directory.
- `./rplay`: runs the binary inside `/bin` (your current dir in host).

Unfortunatelly runtime image is not working, `play` is crashing with

```console
$ docker run --rm -it --user $(id -u):$(id -g) -v $(pwd):/build -w /build rplay-runtime ./rplay
remoteplayer 1 (date.hash)
listenning on tcp://*:23333
media-home: /home/ja/Music
rplay: simple.c:283: snd_mixer_selem_get_playback_volume_range: Assertion `elem' failed.
```

complains.

> TODO: this is not a problem for now, because we only need to deploy, now develop.

## Deploy

### Server side (rplay)

### Docker image

> TODO: describe ...

The first step is to configure `asound.conf` file, we need to pick proper device. Run 

```console
$ aplay -l
**** List of PLAYBACK Hardware Devices ****
card 0: Generic [HD-Audio Generic], device 3: HDMI 0 [HDMI 0]
  Subdevices: 1/1
  Subdevice #0: subdevice #0
card 1: Generic_1 [HD-Audio Generic], device 0: ALC269VC Analog [ALC269VC Analog]
  Subdevices: 1/1
  Subdevice #0: subdevice #0
```

where we are looking for a device with *Master* mixer controler. To list device mixer controllers run

```bash
amixer -c N scontrols
```

command for eaach device, where N is (0, 1, ..., N), e.g.

```console
$ amixer -c 1 scontrols
Simple mixer control 'Master',0
Simple mixer control 'Headphone',0
Simple mixer control 'Speaker',0
Simple mixer control 'PCM',0
Simple mixer control 'Mic',0
Simple mixer control 'Mic Boost',0
Simple mixer control 'Beep',0
Simple mixer control 'Capture',0
Simple mixer control 'Auto-Mute Mode',0
Simple mixer control 'Internal Mic Boost',0
Simple mixer control 'Loopback Mixing',0
```

so we want to pick device 1. Now to pick device 1 modify `assound.conf` this way

```console
$ cat asound.conf 
defaults.pcm.card 1
defaults.ctl.card 1
```


### Manual deployment

- compile with `scons -j11` command
- run remoteplayer with `./rplay /path/to/media/library` command

That is it, remoteplayer (`rplay`) is now listenning on `tcp://*:5557` address.


### Client side (grplay)

> **note**: `grplay` is no longer supported, use arplay (andorid client) instad.

Run `grplay` (graphics remote player client) with rplay IP address and port number

```bash
./grplay 192.168.0.111 3333
```

or run `rplayc` (console remote player client) with rplay IP address, like

```bash
./rplayc 192.168.0.111
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

> TODO: describe rsync configuration I use ...
