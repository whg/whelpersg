#include <chrono>
using namespace std::chrono;

template<class T>
struct _Timer {
	// we're using time_points and durations because VS can't do arithmetic on durations
	time_point<system_clock> startTime;
	T timeTaken;

	void start() {
		startTime = system_clock::now();
	}

	void end() {
		timeTaken = duration_cast<T>(system_clock::now() - startTime);
	}
};

using Timer = _Timer<seconds>;
using SecondsTimer = _Timer<seconds>;
using MillisTimer = _Timer<milliseconds>;

template<class T>
struct _ScopedTimer : public T {
	std::string name;
	_ScopedTimer() = delete; // ScopedTimers must have names
	_ScopedTimer(std::string n) {
		name = n;
		start();
	}

	~_ScopedTimer() {
		end();
		std::cout << name << " took " << timeTaken.count() << "ms" << std::endl;
	}
};

using ScopedTimer = _ScopedTimer<MillisTimer>;
using ScopedSecondsTimer = _ScopedTimer<SecondsTimer>;