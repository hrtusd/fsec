#ifndef TIME_MEASURE_H
#define TIME_MEASURE_H

namespace fsec
{
	typedef std::chrono::high_resolution_clock::time_point TimePoint;
	typedef std::chrono::duration<std::chrono::milliseconds> Duration;
	typedef std::chrono::high_resolution_clock HighResolutionClock;

	class TimeMeasure
	{
	private:
		TimePoint start;
		TimePoint end;
	public:
		void Start();
		void End();
		void Print();
	};

	void TimeMeasure::Start()
	{
		this->start = HighResolutionClock::now();
	}

	void TimeMeasure::End()
	{
		this->end = HighResolutionClock::now();
	}

	void TimeMeasure::Print()
	{
		std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
	}
}
#endif /*TIME_MEASURE_H*/