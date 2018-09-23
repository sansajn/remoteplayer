#pragma once
#include <alsa/asoundlib.h>

//! alsa based volume control implementation
class volume_control
{
public:
	volume_control();
	~volume_control();
	long value();  //!< returns volume value [0, 100] range
	void value(long v);

private:
	long _min, _max;  //!< min/max volume
	snd_mixer_t * _mixer;
	snd_mixer_elem_t * _master_volume;
};
