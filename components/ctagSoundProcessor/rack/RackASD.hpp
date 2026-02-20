#pragma once

#include "RackSynth.hpp"
#include "stmlib/dsp/filter.h"
#include "plaits/dsp/drums/analog_snare_drum.h"

using namespace CTAG::SP;

class RackASD {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	plaits::AnalogSnareDrum asd;
	bool trig_prev {false};
	bool midi_trig {false};
	atomic<int16_t> accent;
	atomic<int16_t> f0;
	atomic<int16_t> tone;
	atomic<int16_t> decay;
	atomic<int16_t> a_spy;
};
