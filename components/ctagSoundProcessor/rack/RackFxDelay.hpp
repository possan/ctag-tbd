#pragma once

#include "RackSynth.hpp"

using namespace CTAG::SP;

class RackFxDelay {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
};
