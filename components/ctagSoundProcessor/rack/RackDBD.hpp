#pragma once

#include "RackSynth.hpp"
#include "stmlib/dsp/filter.h"
#include "stmlib/utils/random.h"
#include "plaits/dsp/drums/synthetic_bass_drum.h"

using namespace CTAG::SP;

class RackDBD {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	plaits::SyntheticBassDrum dbd;
	bool trig_prev {false};
	bool midi_trig {false};
	atomic<int16_t> accent;
	atomic<int16_t> f0;
	atomic<int16_t> tone;
	atomic<int16_t> decay;
	atomic<int16_t> dirty;
	atomic<int16_t> fm_env;
	atomic<int16_t> fm_dcy;
};
