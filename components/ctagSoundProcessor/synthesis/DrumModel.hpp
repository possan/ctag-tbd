#pragma once

#include <iostream>

#define BUF_SZ 32

namespace CTAG::SYNTHESIS{
    class DrumModel {
    public:
        virtual ~DrumModel() {}
        virtual void Init() = 0;
        virtual void Trigger() = 0;
        virtual void Process(float* out, uint32_t size) = 0;
    };
}