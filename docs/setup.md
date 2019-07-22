dokument obsahuje návod ako nastaviť server


použite zariadenie je zotac box (najlacnejší zotac s digitálnym audio výstupom), ďalej inštalovaný systém je kubuntu (v minimálnej inštalácii)

```bash
$ cat /etc/*release
DISTRIB_ID=Ubuntu
DISTRIB_RELEASE=18.04
DISTRIB_CODENAME=bionic
DISTRIB_DESCRIPTION="Ubuntu 18.04.2 LTS"
NAME="Ubuntu"
VERSION="18.04.2 LTS (Bionic Beaver)"
ID=ubuntu
ID_LIKE=debian
PRETTY_NAME="Ubuntu 18.04.2 LTS"
VERSION_ID="18.04"
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
VERSION_CODENAME=bionic
UBUNTU_CODENAME=bionic
```

systém obsahuje audio knižnucu v ~/Music adresáry, ktorú synchronizujem pomocou programu rsync (v základnom móde), bez bežiacej služby, čo znamená, že sa vždy synchronizuje celá knižnica čo je pomalé (>~10min aj v prípade, že pridám len jednu skladbu) 


pozor na súštanie rsyncu, použi -a (archiv mod), ktorý zachová metadata


TODO: 
* ako nastaviť rsyncd ?
* automaticka synchronicázia ?
 




1) Ako sa pripojím k serveru ?


1) Ako sa pripojím k serveru ?

pomocou ssh príkazom

	ssh media@192.168.0.100

kde 192.168.0.100 je IP adresa serveru.

