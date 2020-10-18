# remoteplayer project howto

# Obsah

1-1) Vyzualizácia ZMQ komunikácie ?
1-2) Nastavenie `rplay` serveru pre prístup s AVM (Andorid Virtual Machine) ?



## 1-1) Vyzualizácia ZMQ komunikácie ?

Príkazom

```bash
tcpdump -i any port PORT -A
```

kde `PORT` je číslo portu s `rplay.conf` (pole `rplay.port`), alebo `13333`.

> príkaz nefunguje, zatial neviem prečo

alebo môžem použiť utilitu `rplay_probe.py` (z adreesára `rplay/test`) takto

```bash
python rplay_probe.py --port 13333
```

## 1-2) Nastavenie `rplay` serveru pre prístup s AVM (Andorid Virtual Machine) ?

> describe ...
