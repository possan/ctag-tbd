#pragma once

#include "RackSynth.hpp"
#include "stmlib/dsp/filter.h"
#include "stmlib/utils/random.h"
#include "plaits/dsp/drums/synthetic_snare_drum.h"

using namespace CTAG::SP;

class RackDSD {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	bool midi_trig {false};
	bool trig_prev {false};
	plaits::SyntheticSnareDrum dsd;
	atomic<int16_t> accent;
	atomic<int16_t> f0;
	atomic<int16_t> fm_amt;
	atomic<int16_t> decay;
	atomic<int16_t> spy;
};
