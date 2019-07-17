Prenášanie stavu vs. udalostí, momentálne prenášam udalosti pomocou ktorých sa
v klientovy snažím zrekonštruovať stav

ak by som posielal stav, ten by som na druhej strane nemusel žiadnak
rekonštruovať, jednoducho by som len nastavil príslušné komponenty do správneho
stavu (napr. progress, a tak.)

stav {
	position,
	duration,
	media-playlist-idx,
	playback_state [ready|playing|paused],
}

nevýhoda je, že je potrebné stav prenášať niekoľko krát za sekundu, alebo aspoň
pri každej udalosti. Môžem ale použiť kombinované riešenie v ktorom sa budú
pravidelne posielať stavy (s nízkou frekvenciou opakovania) a prí nejakej
sledovanej udalosti pošlem stav, takže smena sa ku klientom dostane ihneť.

Server tiež musí umožnovať získať obsah playlistu. O playliste uvažujem ako o
dynamicky sa meniacom zozname, takže pri zmene ho zverejním klientom, správou

	playlist_content


Klient na druhej strane volá metódy serveru

	play, pause, stop, seek, identify, list_media, playlist_add, playlist_remove,
volume a ready


Jedinou vecou na vyžiadanie je knižnica, metóda

	list_media


{}

Ako identifikovať song ?

* podľa cesty - tá sa ale môžem meniť (napr. pri presunutí súboru, alebo pri presune celej knižnice)

* podľa hash súčtu - pri zmene ID tagu sa zmení aj hash súboru

to čo potrebujem je skombinovať oba spôsoby

* ak narazím na nový súbor spočítam hash a skontrolujem či sa už náhodou nenachádza v knižnici. Ak áno a súbor neexistuje na danej pozícii, priradím nový súbor k už existujúcemu záznamu v knižnici. Ak hash nenájdem, potom sa jedná o nový song. - tímto spôsobom viem rozpoznať presun a duplicitný song.





done - * rplay: version
* rplay: implementuj queue_change
done - * rplay:	implementuj play_progress
* grplay: play_progress


(?) čo keby som zaviedol eventový systém založený na boost asio ?

(?) Ako hladať v knižnici ?

zatiľ pomocou regex searchu s názvu súboru, uvidím ako to bude
fungovať

audio, video, media, content

rplay


https://commons.wikimedia.org/wiki/Comparison_of_icon_sets
