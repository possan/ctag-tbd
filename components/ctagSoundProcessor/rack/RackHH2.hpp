#pragma once

#include "RackSynth.hpp"
#include "plaits/dsp/drums/hi_hat.h"

using namespace CTAG::SP;

class RackHH2 {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	plaits::HiHat<plaits::RingModNoise, plaits::LinearVCA, false, true> hh2;
	bool trig_prev {false};
	bool midi_trig {false};
	float temp1[BUF_SZ];
	float temp2[BUF_SZ];
	atomic<int16_t> accent;
	atomic<int16_t> f0;
	atomic<int16_t> tone;
	atomic<int16_t> decay;
	atomic<int16_t> noise;
};
