#pragma once

#include "RackSynth.hpp"
#include "synthesis/Clap.hpp"

using namespace CTAG::SP;

class RackClap {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	CTAG::SYNTHESIS::Clap cl;
	bool trig_prev {false};
	bool midi_trig {false};
	atomic<int16_t> f0;
	atomic<int16_t> tone;
	atomic<int16_t> decay;
	atomic<int16_t> scale;
	atomic<int16_t> transient;
};
