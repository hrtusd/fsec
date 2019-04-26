#include "pch.h"

namespace fsec {
    TimePoint timer_timepoint() {
        return HighResolutionClock::now();
    }

    void timer_print(TimePoint tp1, TimePoint tp2) {
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(tp2 - tp1).count() << " ms" << std::endl;
    }
}