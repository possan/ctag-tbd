#pragma once

#include "RackSynth.hpp"

using namespace CTAG::SP;

class RackChannelMixer {
public:
	void PreProcess(const PicoSeqRackProcessData &data);
	void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	int device;
	float level;
	float pan;
	float send1;
	float send2;
	int cc_base;
	int track_length;

private:
	atomic<int16_t> mix_lev;
	atomic<int16_t> mix_device;
	atomic<int16_t> mix_pan;
	atomic<int16_t> mix_fx1;
	atomic<int16_t> mix_fx2;
};
