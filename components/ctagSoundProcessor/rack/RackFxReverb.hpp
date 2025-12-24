#pragma once

#include "RackSynth.hpp"

using namespace CTAG::SP;

class RackFxReverb {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
};
