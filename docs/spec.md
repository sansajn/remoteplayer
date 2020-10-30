# Špecifikácia Remote Player Protokolu, verzia 0.6

RPP (**R**emote **P**layer **P**rotocol) je určený pre komunikáciu klentou z audio serverom.

## server

Umožnuje klientom prehrávať média z knižnice medií. Server implementuje nasledujúce moduly

player: jednoduchý blokujúci playbin (GST), ktorý môže napr. prehrávať zo zásobníka.

library: modul dodá všetky fajly s adresára

interface: odošle klientovy zoznam medií, umožní klientovy prehrať medium z
knižnice a po pripojení pošle klientom `playlist_content` a ak sa prehráva
skladba, potom aj `play_progress` správu.

interface bude implementovaný pomocou ZMQ ako clone pattern client/server a
bude poskytovať nasledujúce funkcie:

* `list_media`, ktorá vráti zoznam medií
* `play`, ktorá prehrá skladbu z playlistu
* `pause`, ktorá pozastaví prehrávanie aktuálnej skladby
* `stop`, zastavenie prehrávania
* `play_progress`, update priebehu prehrávania (každých 10s)
* `identify`, vráti informácie o serveru
* `playlist_content`, update obsahu playlistu
* `playlist_add`, pridáva skladbu do playlistu
* `playlist_remove`, odobera skladbu s playlistu
* `playlist_move`, zmena pozície skladby v playliste
* `playlist_shuffle`, zapne/vypne shuffle mód
* `alive`, server posiela klientom kazdu 1s (niečo ako ping)
* `seek`, ktorá posunie aktualnu skladbu na požadovanú pozíciu
* `volume`, posiela server pri zmene hlasitosti
* `set_volume`, zmena hlasitosti
* `client_ready`, signal serveru, ze je klient pripraveny prijímať správy
* `bed_time`, zapne/vypne bed time mód
* `library_update`, posiela server v prípade zmeny v knižnici 
* `download`, posiela klient s požiadavkou o stiahnutie skladby
* `download_progress`, posiela server s informaciou o stavu stahovania

Správu `play_progress` môže GUI odchitiť a zobraziť informácie o prehrávanom
obsahu.

configuration: umožnuje konfigurovať voľby ako port a media home

{{version}}

Build bude označený verziou a dostupný bude aj commit number, server bude tiež
zobrazovať aktuálnu prehrávaciu frontu.


## klient

Pripojí sa k serveru a požiada o zoznam medií, zobrazí ich a umoží
užívateľovy výber pomocou GUI. GUI bude obsahovať list-box a regex
search entry, z listboxu bude možné vybrať skladbu a pomocou tlačítka
požiadať o jej prehratie. GUI bude tiež obsahovať tlačítko pre
zastavenie prehrávania skladby. Skladby budú v list-boxe zobrazené ako
strom.

Ďalej klient zobrazí aktuálne prehrávanú skladbu a jej priebeh (časovú
os s označenou pozíciou prehrávania).

Zo serveru získa obsah prehrávacej fronty a jednotlivé skladby označí
poradovým číslom (1, 2, ..., N) v zozname skladieb (list-boxe).


### interface API

`list_media` [ask]:

```js
{"cmd":"list_media"}
```

Umožnuje klientovy získať zoznam medií (knižnicu), odpoveď klientovy vypadá
takto

```js
{"cmd":"media_library", "items":[
	"path/to/file1",
	"path/to/file2",
	"path/to/file3"
]}
```

Knižnica sú jediné dáta na ktoré sa musí klient explicitne opýtať a to kvôly
tomu, že predpokladám tisícky prvkou v knižnici, takže by prípadný oznam mohol
byť veľký aj stovky KB.


play [notify]:

Príkaz umožnuje clientovy prehrať skladbu z playlistu v tvare

	{
		"cmd":"play", 
		"playlist":PID,  // size_t
		"idx":N          // size_t
	}

kde PID je cele kladne číslo (size_t) reprezentujúce playlist a N (size_t) je
index skladby v playliste.


pause [notify|news]:

Príkaz unožnuje klientovy pozastaviť prehrávanie aktuálnej skladby, príkaz je v
tvare

	{"cmd":"pause"}

následne server klientom pošle správu play_progress(playback_status=paused).


stop [notify]:

Príkaz umožnuje klientovy zastaviť prehrávanie, príkaz je definovaný
ako

	{"cmd":"stop"}

následne server klientom pošle správu play_progress(media="").


play_progress [news]:

Správu posiela server v pravidelných intervaloch (momentálne 10s) počas
prehrávania skladby. Správa je definovaná ako

	{
		"cmd":"play_progress",   // string
		"position":P,            // long
		"duration":D,            // long
		"playlist_id":N,         // size_t
		"media_idx":N,           // size_t
		"playback_state":S,      // int (playing=1, paused=2, stopped=3)
		"mode":M                 // int (none=0, shuffle=1, bed_time=2)
	}

kde `position` je aktuálna pozícia v nano-sekundách, `duration` je dĺžka skladby v
nano-sekundách, `playlist_id` je identifikátor playlistu a `media_idx` je index
aktuálnej skladby v playliste `playlist_id`. Pole `playback_state` predstavuje
prehrávací stav, ktorý je buď v stave `playing` (1), `paused` (2), alebo v
stave `stopped` (3) a pole `mode`, ktoré je zoznamom módou playlistu (kombinácie
módou `shuffle` (1) a `bed_time` (2)).

note: position a duration zvyknem v klientovy nazývať time a length, porozmýšlaj o zmene na servery.

Ak klient obdrží play_progress v ktorej media je prázdny reťazec, potom
remoteplayer signalizuje stop (pozri popis stop príkazu).


identify [ask]:

Správa/dotaz identify umožnuje klientovi získať informácie o serveru ako napr.
verziu

	{'cmd':'identify'}

server na dotaz odpovie takto

	{'cmd':'server_desc', 'version':'0.2.0', 'build':'417a36c'}


playlist_content [news]:

Správou informuje server klientou o zmene obsahu playlistu. Správa je
definovaná takto:

	{
		'cmd':'playlist_content',
		'id':N,                          // size_t
		'items':['item1', 'item2', ...]
	}

kde N, identyfikátor playlistu je celé kladné číslo a items obsahuje pole reťazcou, kde jednotlive reťazece popisujú položky playlistu (v aktuálnej implementácii sa jedná o názvy súborou).


alive [news]:

remoteplayer posiela príkaz v pravidelných intervaloch (každú 1s) v tvare

	{
		'cmd':'alive',
		'count':N,
		'time_stamp':ISO_STRING
	}

kde N je čítač a ISO_STRING je časové razítko v tvare YYYYMMDDTHHMMSS.ffffff
reprezentované reťazcom.


seek [notify]:

klient posielá správu na zmenu pozície aktuálnej skladby v tvare

	{
		"cmd":"seek",
		"position":POS_IN_SEC,
		"media":"/path/to/file"
	}

kde POS_IN_SEC (int) je nová pozícia skladby a media je identifikátor skladby (ak sa identifikátor nezhoduje s identifikátorom aktuálne prehrávanej skladby, správa sa ignoruje).


volume [news]:

posiela server pri zmene hlasitosti v tvare

	{"cmd":"volume", "value":N}

kde N (int) je celé kladné číslo v rozsahu 0 až 100. Server volume spravu
posiela aj pri pripojeni klienta k serveru.


set_volume [notify]:

Príkaz užívateľovi zmenu hlasitosti

	{"cmd":"set_volume",	"value":N}

kde N (int) je celé kladné číslo v rozsahu 0 až 100.


playlist_add [notify]:

Umožnuje klientovy pridať skladbu do playlistu, správa je v tvare

	{"cmd":"playlist_add", "items":["path/to/file1", ...]}


playlist_remove [notify]:

Umožnuje odobrať skladbu z playlistu v tvare

	{"cmd":"playlist_remove", "playlist":PID, "items":[i1, i2, ..., iN]}

kde PID (size_t) je identifikátor playlistu a items je zoznam indexou skladieb
playlistu, ktoré sa majú odstrániť. 

Následne ako reakcia na zmenu playlistu server vyheneruje správu
playlist_content.


`playlist_move` [notify]:

Príkaz umožnuje zmenu pozície skladby v playliste a je definovaný ako

	{"cmd":"playlist_move", "playlist":PID, "from":N, "to":M}

kde PID (`size_t`) je identifikátor playlistu, `N` index skladby a `M` je nový index skladby.


`playlist_shuffle` [notify]:

Príkaz zapne/vypne shuffle mód a je definovaný takto

	{"cmd":"playlist_shuffle", "shuffle":S}

kde S je hodnota typu bool, true ak ma byť shuffle mód zapnutý, false inak.


`bed_time` [notify]:

Príkaz zapne/vypne bed time mód (zastavenie prehrávania po skončení aktuálnej skladby) a je definovaný takto

	{"cmd":"bed_time", "value":V}

kde V je hodnota typu bool, true ak ma byť bed time mód zapnutý, false inak.


`library_update` [news]:

Posiela server klientom v prípade zmeny v knižnici (nový, alebo zmazaný súbor) v tvare

	{"cmd":"library_update"}


`client_ready` [notify]:

Umožnuje klientom signalizovať, že sú od serveru pripravený príjimať správy, po obdržaní `client_ready` server pošle správy `playlist_content` a `play_progress` v prípade, že niečo prehráva.

	{"cmd":"client_ready"}


V prípade neznámeho dotazu odpovie server správou

	{"cmd":"error", "what":"unknown <CMD> command"}


`download` [notify]:

Požiadavka na stiahnutie skladby z internetu (bude implementované na strane servera pomocou utility `youtube-dl`) v tvare

```js
{"cmd":"download", "url":URL}
```

kde *URL* je reťazec (string). Po začatí stahovania server začne posielať správy `download_progress`.



`download_progress` [news]:

Posiela server po začatí sťahovania s informaciou o stavu stahovania v tvare

```js
{"cmd":"download_progress", "items":[
	{"n":"item1", "p":P1}, 
	{"n":"item2", "p":P2}, 
	...]}
```

kde pole `items` je zoznam dvojíc nesúcich názov skladby ako `n` a koľko je zo skladby už stiahnuté ako pole `p`. `Pn` je číslo v rozsahu 0 až 100, kde 100 znamená, že skladba je stiahnutá. 

Dokončenie sťahovania server signalizuje dvojicou `{"n":"item", "p":100}`, nasledujúca správa `download_progress` už dvojicu obsahovať nebude.

