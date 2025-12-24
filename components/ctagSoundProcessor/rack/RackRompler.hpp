#pragma once

#include "RackSynth.hpp"
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "helpers/ctagSampleRom.hpp"

using namespace CTAG::SP;

class RackRompler {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float s1_out[BUF_SZ];
	void noteOn(uint8_t note, uint8_t vel);
	void noteOff(uint8_t note, uint8_t vel);

private:
	CTAG::SYNTHESIS::RomplerVoiceMinimal rompler;
	bool trig_prev {false};
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};
	atomic<int16_t> s1_speed;
	atomic<int16_t> s1_pitch;
	atomic<int16_t> s1_bank;
	atomic<int16_t> s1_slice;
	atomic<int16_t> s1_start;
	atomic<int16_t> s1_end;
	atomic<int16_t> s1_lp;
	atomic<int16_t> s1_lp_pp;
	atomic<int16_t> s1_lp_pos;
	atomic<int16_t> s1_atk;
	atomic<int16_t> s1_dcy;
	atomic<int16_t> s1_eg2fm;
	atomic<int16_t> s1_brr;
	atomic<int16_t> s1_ft;
	atomic<int16_t> s1_fc;
	atomic<int16_t> s1_fq;
	atomic<int16_t> s1_tsmode;
	atomic<int16_t> s1_tsamount;
	atomic<int16_t> s1_tssteps;
};
