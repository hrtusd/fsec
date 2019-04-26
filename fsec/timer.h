#ifndef TIME_MEASURE_H
#define TIME_MEASURE_H

namespace fsec
{
	typedef std::chrono::high_resolution_clock::time_point TimePoint;
	typedef std::chrono::high_resolution_clock HighResolutionClock;

    TimePoint timer_timepoint();
    void timer_print(TimePoint tp1, TimePoint tp2);
}
#endif /*TIME_MEASURE_H*/