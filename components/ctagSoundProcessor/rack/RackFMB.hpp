#pragma once

#include "RackSynth.hpp"
#include "synthesis/FmKick.hpp"

using namespace CTAG::SP;

class RackFMB {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	void trigger();

private:
	CTAG::SYNTHESIS::FmKick fmb;
	bool trig_prev {false};
	bool midi_trig {false};
	atomic<int16_t> use_ratio_mode;
	atomic<int16_t> mod_env_sync;
	atomic<int16_t> f_b;
	atomic<int16_t> d_b;
	atomic<int16_t> f_m;
	atomic<int16_t> I;
	atomic<int16_t> d_m;
	atomic<int16_t> b_m;
	atomic<int16_t> A_f;
	atomic<int16_t> d_f;
};
