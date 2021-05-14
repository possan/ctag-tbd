#include <atomic>
#include "ctagSoundProcessor.hpp"

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorVoid : public ctagSoundProcessor {
        public:
            virtual void Process(const ProcessData &) override;
            ctagSoundProcessorVoid();
            virtual ~ctagSoundProcessorVoid();

        private:
            virtual void knowYourself() override;

            // private attributes could go here
            // autogenerated code here
            // sectionHpp
	atomic<int32_t> dummy, trig_dummy;
	// sectionHpp
        };
    }
}