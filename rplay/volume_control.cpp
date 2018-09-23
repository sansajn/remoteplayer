#include "volume_control.hpp"

volume_control::volume_control()
	: _min{0}, _max{0}, _mixer{nullptr}, _master_volume{nullptr}
{
	char const * card = "default";
	char const * selem_name = "Master";

	snd_mixer_open(&_mixer, 0);
	snd_mixer_attach(_mixer, card);
	snd_mixer_selem_register(_mixer, nullptr, nullptr);
	snd_mixer_load(_mixer);

	snd_mixer_selem_id_t * sid;
	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	_master_volume = snd_mixer_find_selem(_mixer, sid);

	snd_mixer_selem_get_playback_volume_range(_master_volume, &_min, &_max);
}

volume_control::~volume_control()
{
	snd_mixer_close(_mixer);
}

long volume_control::value()
{
	long vol;
	snd_mixer_selem_get_playback_volume(_master_volume, SND_MIXER_SCHN_FRONT_LEFT, &vol);
	return (((double)vol + 0.5) * 100.0) / (double)_max;
}

void volume_control::value(long v)
{
	if (v >= 0 && v <= 100)
		snd_mixer_selem_set_playback_volume_all(_master_volume, (v * _max) / 100);
}
