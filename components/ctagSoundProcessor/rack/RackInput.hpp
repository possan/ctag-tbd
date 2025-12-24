#pragma once

#include "RackSynth.hpp"

using namespace CTAG::SP;

class RackInput {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float out[BUF_SZ * 2];
};
