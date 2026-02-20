#pragma once

#include "RackSynth.hpp"
#include "stmlib/dsp/filter.h"
#include "plaits/dsp/drums/analog_bass_drum.h"

using namespace CTAG::SP;

class RackABD {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	bool trig_prev {false};
	bool midi_trig {false};
	plaits::AnalogBassDrum abd;
	atomic<int16_t> accent;
	atomic<int16_t> f0;
	atomic<int16_t> tone;
	atomic<int16_t> decay;
	atomic<int16_t> a_fm;
	atomic<int16_t> s_fm;
};
