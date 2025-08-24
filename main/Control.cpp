/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.
(c) 2023 MIDI-Message-Parser aka 'bba_update()' by Mathias Brüssel. 

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "Control.hpp"
#include "Calibration.hpp"
#include "gpio.hpp"
#include "adc.hpp"

#include "rp2350_spi_stream.hpp"
#define BUF_SZ (N_CVS*4 + N_TRIGS)

uint8_t *CTAG::CTRL::Control::buf_ptr = nullptr; // buffer pointer for current cv + trig data
uint16_t updatecounter = 0;

IRAM_ATTR void CTAG::CTRL::Control::Update(uint8_t *trigs, float *cvs, uint32_t ledStatus) {
    if (CTAG::DRIVERS::rp2350_spi_stream::GetCurrentBuffer(&buf_ptr, BUF_SZ, ledStatus)) {

        if (updatecounter % 3000 == 0) {
            ESP_LOGI("Control", "Got SPI update %d%d%d%d%d%d%d%d",
                buf_ptr[N_CVS * 4 + 0],
                buf_ptr[N_CVS * 4 + 1],
                buf_ptr[N_CVS * 4 + 2],
                buf_ptr[N_CVS * 4 + 3],
                buf_ptr[N_CVS * 4 + 4],
                buf_ptr[N_CVS * 4 + 5],
                buf_ptr[N_CVS * 4 + 6],
                buf_ptr[N_CVS * 4 + 7]);
        }

        updatecounter ++;

        memcpy(cvs, buf_ptr, N_CVS * 4);
        memcpy(trigs, &buf_ptr[N_CVS * 4], N_TRIGS);
    }
}

void CTAG::CTRL::Control::SetCVChannelBiPolar(const bool &v0, const bool &v1, const bool &v2, const bool &v3) {
    // ifdefs to exclude this from BBA and MK2 are in Calibration.hpp
    CTAG::CAL::Calibration::ConfigCVChannels(v0 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v1 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v2 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar,
                                             v3 ? CTAG::CAL::CVConfig::CVBipolar : CTAG::CAL::CVConfig::CVUnipolar);
}

void CTAG::CTRL::Control::Init() {
    ESP_LOGI("Control", "Initializing control! %d CVs, %d Trigs", N_CVS, N_TRIGS);
    buf_ptr = DRIVERS::rp2350_spi_stream::Init();
}