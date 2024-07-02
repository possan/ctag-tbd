
#pragma once

#include <atomic>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
// #include "CalibrationModel.hpp"

namespace CTAG {
    namespace CTRL {
        class CPUDebug final {
        public:
            CPUDebug() = delete;
            static void Init();
            // static void FlushBuffers();
            // static void SetCVChannelBiPolar(bool const &v0, bool const &v1, bool const &v2, bool const &v3);
            // IRAM_ATTR static void Update(uint8_t **trigs, float **cvs);
        private:
            static void task(void *params);
            static TaskHandle_t taskHandle;
        };
    }
}