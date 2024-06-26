/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include <atomic>
#include "ctagSoundProcessor.hpp"
#include "airwindows/EChorus.hpp"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorEChorus : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;

           virtual void Init(std::size_t blockSize, void *blockPtr) override;

            virtual ~ctagSoundProcessorEChorus();

        private:
            virtual void knowYourself() override;

            float preRange = 0.f;

            //default stuff
            airwindows::EChorus echorus;
            // private attributes could go here
            // autogenerated code here
            // sectionHpp
            atomic<int32_t> bypass, trig_bypass;
            atomic<int32_t> pdepth, cv_pdepth;
            atomic<int32_t> stages, cv_stages;
            atomic<int32_t> prate, cv_prate;
            atomic<int32_t> pwidth, cv_pwidth;
            atomic<int32_t> pwet, cv_pwet;
            atomic<int32_t> mono, trig_mono;
            // sectionHpp
        };
    }
}