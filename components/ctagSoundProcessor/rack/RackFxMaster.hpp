#pragma once

#include "RackSynth.hpp"

using namespace CTAG::SP;

class RackFxMaster {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
};
