Scripts:

Python scripts us pyzmq library for ZMQ communication, see lybrary documentation at https://pyzmq.readthedocs.io/en/latest/.

To create virtual python environment with all dependencies, run

```bash
python3 -m venv /opt/venvs/ja/rplay
source /opt/venvs/ja/rplay/bin/activate
python3 -m pip install -r requirements.txt
```

for the first time. After that only activation with

```bash
source /opt/venvs/ja/rplay/bin/activate
```

> **tip**: or use helper script `./activate` instead

command is needed.

- `rplay_probe.py`: python script to brobe rplay communication with client. To run the script join to running development container with

```bash
make join
```

command from `rplay` directory.
