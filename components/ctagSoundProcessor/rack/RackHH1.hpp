#pragma once

#include "RackSynth.hpp"
#include "plaits/dsp/drums/hi_hat.h"

using namespace CTAG::SP;

class RackHH1 {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	plaits::HiHat<plaits::SquareNoise, plaits::SwingVCA, true, false> hh1;
	bool trig_prev {false};
	bool midi_trig {false};
	float temp1[BUF_SZ];
	float temp2[BUF_SZ];
	atomic<int16_t> hh1_accent;
	atomic<int16_t> hh1_f0;
	atomic<int16_t> hh1_tone;
	atomic<int16_t> hh1_decay;
	atomic<int16_t> hh1_noise;
};
