#include <jni.h>
#include <android/log.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

static string to_string(JNIEnv * env, jstring & jstr);
static vector<string> to_vector(JNIEnv * env, jobject & items);
static jobject to_ArrayList(JNIEnv * env, vector<string> const & items);
static jstring to_String(JNIEnv * env, string const & str);


class rplay_client
{
public:
	static rplay_client & ref();
	~rplay_client();
	// open()
	void play(size_t playlist_id, size_t playlist_idx);
	void pause();
	void stop();
	void seek(size_t position, string const & media);
	void prev();
	void next();
	void volume(int value);
	void playlist_add(vector<string> const & items);
	void playlist_remove(size_t playlist_id, size_t playlist_idx);
	void ask_identity();
	void ask_library();
	void close();

	vector<string> library();
	vector<string> playlist();
	string media();
	size_t position();
	size_t duration();

private:
	static rplay_client * _instance;
};

rplay_client * rplay_client::_instance = nullptr;


rplay_client & rplay_client::ref()
{
	if (!_instance)
		_instance = new rplay_client;
	return *_instance;
}

rplay_client::~rplay_client()
{
	close();
}

void rplay_client::play(size_t playlist_id, size_t playlist_idx)
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "play()");
}

void rplay_client::pause()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "pause()");
}

void rplay_client::stop()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "stop()");
}

void rplay_client::seek(size_t position, string const & media)
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "seek()");
}

void rplay_client::prev()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "prev()");
}

void rplay_client::next()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "next()");
}

void rplay_client::volume(int value)
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "volume()");
}

void rplay_client::playlist_add(vector<string> const & items)
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "playlist_add()");
}

void rplay_client::playlist_remove(size_t playlist_id, size_t playlist_idx)
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "playlist_remove()");
}

void rplay_client::ask_identity()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "ask_identity()");
}

void rplay_client::ask_library()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "ask_library()");
}

void rplay_client::close()
{
	__android_log_print(ANDROID_LOG_DEBUG, "arplay", "%s", "close()");

	delete _instance;
	_instance = nullptr;
}

vector<string> rplay_client::library()
{
	return vector<string>{{
		"/home/adam/Music/2018/Magdalena @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-Xq0d81xOSAs.m4a",
		"/home/adam/Music/2018/Solomun   Diynamic, Blue Parrot The BPM Festival 2015, Mexico-IsC9S_Qy84E.m4a",
		"/home/adam/Music/2018/Solomun b2b Dixon - Live @ Exit Festival July 2017-332496388.mp3",
		"/home/adam/Music/2018/100 Days until Tomorrowland _ Paul Kalkbrenner - LIVE-wMFy7vgfBFc.opus",
		"/home/adam/Music/2018/Adriatique @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-5eI8H85eedQ.opus",
		"/home/adam/Music/2018/Adriatique @ Neopop Electronic Music Festival 2018 (BE-AT.TV)-DPr7vl15qo0.opus",
		"/home/adam/Music/2018/Amelie Lens @ LaPlage de Glazart for Cercle-1q-1Bpy168g.opus",
		"/home/adam/Music/2018/Andrea Oliva _ Tomorrowland Belgium 2018-LonYe1UcR34.opus",
		"/home/adam/Music/2018/Andy Bros @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-iJT1Ag1S4_Y.opus",
		"/home/adam/Music/2018/ANNA - Rave On Snow 2017 (BE-AT.TV)-7LNsnWbLcFU.opus",
		"/home/adam/Music/2018/ANNA vinyl DJ set @ Les Pavillons des Etangs for Cercle-O8x2WIxpnYg.opus",
		"/home/adam/Music/2018/Armin van Buuren live at AFAS Live - A State Of Trance 836 (ADE 2017 Special)-t3R-TSm5pfw.opus",
		"/home/adam/Music/2018/Awakenings Festival 2018 Sunday - Liveset Amelie Lens @ Area V-5Cs4NqGC8jA.opus",
		"/home/adam/Music/2018/BARCELONA deep house session FEBRUARY 2018-PJJ9YK_McFU.opus",
		"/home/adam/Music/2018/Black Coffee _ Tomorrowland Belgium 2018-B3ObxaSCY9Y.opus",
		"/home/adam/Music/2018/Boris Brejcha @ Tomorrowland Belgium 2018-buCD-_1UPn4.opus",
		"/home/adam/Music/2018/Camo & Krooked @ Monumental 2018 Feat. Together Barcelona (BE-AT.TV)-Td0NKg2gGZM.opus",
		"/home/adam/Music/2018/Charlotte de Witte at Pukkelpop 2018 (Main Stage)-sy0t9xoTdBU.opus",
		"/home/adam/Music/2018/Charlotte de Witte @ Dour Festival 2017 on Cercle-AtZxg-OC5ho.opus",
		"/home/adam/Music/2018/Charlotte de Witte @ La Rotonde Stalingrad for Cercle-6EfxIM8ZnTk.opus",
		"/home/adam/Music/2018/CHARLOTTE DE WITTE & LEDISKO in The Lab NYC [Turbo Recordings Showcase ]-5-SOxDlj_c8.opus",
		"/home/adam/Music/2018/Charlotte De Witte Live at New Horizons Festival 2017-bQ_NV2iCcVU.opus",
		"/home/adam/Music/2018/CHARLOTTE DE WITTE techno set at CRSSD Fest _ Spring 2018-ks2vQsoHDgU.opus",
		"/home/adam/Music/2018/Charlotte de Witte _ Tomorrowland Belgium 2018-mWvdsMwnGmM.opus",
		"/home/adam/Music/2018/Deborah De Luca @ Alltimeclubbing Bucharest (BE-AT.TV)-TwON9KsqyDk.opus",
		"/home/adam/Music/2018/Deep Emotions - Paul Kalkbrenner • Boris Brejcha • Carl Cox • Oxia ' Vol.2-mrxlBr1stSY.opus",
		"/home/adam/Music/2018/ERICK MORILLO in The Lab LDN-_Bka0eX1UHk.opus",
		"/home/adam/Music/2018/HOSH @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-DEwV2vx9FRY.opus",
		"/home/adam/Music/2018/Kollektiv Turmstrasse _ Tomorrowland Belgium 2018-0Q3CIkx6ho4.opus",
		"/home/adam/Music/2018/Kölsch @ Awakenings Easter Special 2018 (BE-AT.TV)-gkI28sWc9GY.opus",
		"/home/adam/Music/2018/Lehar _ Tomorrowland Belgium 2018-u4NbJWPBQe8.opus",
		"/home/adam/Music/2018/Melodic Techno Mix 2018 Solomun , Boris Brejcha , Adrien Kepler , Max Tenrom , Ben C & Kalsx vol 27-3X6fewEI080.opus",
		"/home/adam/Music/2018/Monika Kruse @ Montparnasse Tower Observation Deck for Cercle-oukwTJ81Zp0.opus",
		"/home/adam/Music/2018/Pan-Pot @ Awakenings Easter Special 2018 (BE-AT.TV)-3AqWBMEHCxQ.opus",
		"/home/adam/Music/2018/Paul Kalkbrenner @ Zurich Street Parade 2018 (BE-AT.TV)-nuh5B4aRsck.opus",
		"/home/adam/Music/2018/Sander van Doorn Live at Ultra Music Festival (Miami, USA) 03.30.2014-mDLcj6y9eko.opus",
		"/home/adam/Music/2018/Solomun - Boris Brejcha - Stephan Bodzin - Tale Of Us - Pan Pot-ZnnhTzOhJDk.opus",
		"/home/adam/Music/2018/Solomun DJ set @ Diynamic Outdoor - Off Week Barcelona 2018 (BE-AT.TV)-No6pJpR6gwk.opus"}};
}

vector<string> rplay_client::playlist()
{
	return vector<string>{{
		"Amelie Lens @ LaPlage de Glazart for Cercle-1q-1Bpy168g.opus",
		"Adriatique @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-5eI8H85eedQ.opus",
		"HOSH @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-DEwV2vx9FRY.opus",
		"Kölsch @ Awakenings Easter Special 2018 (BE-AT.TV)-gkI28sWc9GY.opus",
		"Solomun @ Kappa FuturFestival 2018-KYcEL11X8ZU.opus"}};
}

string rplay_client::media()
{
	return "Adriatique @ Diynamic Outdoor - Off Week 2018 (BE-AT.TV)-5eI8H85eedQ.opus";
}

size_t rplay_client::position()
{
	return 1*60+23;
}

size_t rplay_client::duration()
{
	return 43*60+56;
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_remoteplayer_arplay_PlayerActivity_stringFromJNI(
		JNIEnv* env,
		jobject /* this */)
{
	string hello = "Hello from C++";
	return env->NewStringUTF(hello.c_str());
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_play(
		JNIEnv* env,
		jobject /* this */,
		jlong playlist_id,
		jlong playlist_idx)
{
	rplay_client::ref().play((size_t)playlist_id, (size_t)playlist_idx);
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_pause(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().pause();
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_stop(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().stop();
}

string to_string(JNIEnv * env, jstring & jstr)
{
	char const * str = env->GetStringUTFChars(jstr, nullptr);
	string result{str};
	env->ReleaseStringUTFChars(jstr, str);
	return result;
}

vector<string> to_vector(JNIEnv * env, jobject & items)
{
	jclass java_util_ArrayList = env->FindClass("java/util/ArrayList");
	jmethodID java_util_ArrayList_ = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
	jmethodID java_util_ArrayList_size = env->GetMethodID(java_util_ArrayList, "size", "()I");
	jmethodID java_util_ArrayList_get = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");

	vector<string> result;
	jint size = env->CallIntMethod(items, java_util_ArrayList_size);
	for (jint i = 0; i < size; ++i)
	{
		jstring item = (jstring)env->CallObjectMethod(items, java_util_ArrayList_get, i);
		result.push_back(to_string(env, item));
	}

	return result;
}

jobject to_ArrayList(JNIEnv * env, vector<string> const & items)
{
	jclass java_util_ArrayList = env->FindClass("java/util/ArrayList");
	jmethodID java_util_ArrayList_ = env->GetMethodID(java_util_ArrayList, "<init>", "(I)V");
	jmethodID java_util_ArrayList_add = env->GetMethodID(java_util_ArrayList, "add", "(Ljava/lang/Object;)Z");

	jobject result = env->NewObject(java_util_ArrayList, java_util_ArrayList_, items.size());
	for (string const & s : items)
	{
		jstring element = env->NewStringUTF(s.c_str());
		env->CallBooleanMethod(result, java_util_ArrayList_add, element);
		env->DeleteLocalRef(element);
	}

	return result;
}

jstring to_String(JNIEnv * env, string const & str)
{
	return env->NewStringUTF(str.c_str());
}


extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_seek(
		JNIEnv* env,
		jobject /* this */,
		jlong position,
		jstring media)
{
	rplay_client::ref().seek((size_t)position, to_string(env, media));
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_prev(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().prev();
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_next(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().next();
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_volume(
		JNIEnv* env,
		jobject /* this */,
		jint value)
{
	rplay_client::ref().volume((int)value);
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_playlistAdd(
		JNIEnv* env,
		jobject /* this */,
		jobject items)
{
	rplay_client::ref().playlist_add(to_vector(env, items));
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_playlistRemove(
		JNIEnv* env,
		jobject /* this */,
		jlong playlist_id,
		jlong playlist_idx)
{
	rplay_client::ref().playlist_remove((size_t)playlist_id, (size_t)playlist_idx);
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_askIdentity(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().ask_identity();
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_askLibrary(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().ask_library();
}

extern "C" JNIEXPORT void JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_close(
		JNIEnv* env,
		jobject /* this */)
{
	rplay_client::ref().close();
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_media(
		JNIEnv* env,
		jobject /* this */)
{
	return to_String(env, rplay_client::ref().media());
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_position(
		JNIEnv* env,
		jobject /* this */)
{
	return (jlong)rplay_client::ref().position();
}

extern "C" JNIEXPORT jlong JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_duration(
		JNIEnv* env,
		jobject /* this */)
{
	return (jlong)rplay_client::ref().duration();
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_playlist(
		JNIEnv* env,
		jobject /* this */)
{
	return to_ArrayList(env, rplay_client::ref().playlist());
}

extern "C" JNIEXPORT jobject JNICALL
Java_com_remoteplayer_arplay_DummyRPlayClient_library(
		JNIEnv* env,
		jobject /* this */)
{
	return to_ArrayList(env, rplay_client::ref().library());
}
