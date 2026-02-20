#pragma once

#include "RackSynth.hpp"
#include "synthesis/Rimshot.hpp"

using namespace CTAG::SP;

class RackRimshot {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float rs_out[32];
	void trigger();

private:
	CTAG::SYNTHESIS::Rimshot rs;
	bool trig_prev {false};
	bool midi_trig {false};
	atomic<int16_t> accent;
	atomic<int16_t> f0;
	atomic<int16_t> tone;
	atomic<int16_t> decay;
	atomic<int16_t> noise;
};
