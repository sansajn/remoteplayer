Python scripts use pyzmq library for ZMQ communication, see lybrary documentation at https://pyzmq.readthedocs.io/en/latest/.

To create virtual python environment with all dependencies installed, run

```bash
python3 -m venv /opt/venvs/ja/rplay
source /opt/venvs/ja/rplay/bin/activate
python3 -m pip install -r requirements.txt
```

for the first time. After that only activation with

```bash
source /opt/venvs/ja/rplay/bin/activate
```

> **tip**: or use helper script `source ./activate` instead

command is needed.

To support debugging within docker containers, because that is how rplay is meant to be deployed we can use

```bash
make join
```

command from `rplay` directory to open additional bash inside runnning container.

**Scripts**:

- `rplay_probe.py`: python script to probe rplay communication with client.

- `play.py`: script adds the first item from library into playlist and then starts playback.

- `stop.py`: stops playback by sending stop command
