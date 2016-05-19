#include <chrono>

namespace whg {

template<class T>
struct _Timer {
	// we're using time_points and durations because VS can't do arithmetic on durations
    std::chrono::time_point<std::chrono::system_clock> startTime;
	T timeTaken;

	void start() {
		startTime = std::chrono::system_clock::now();
	}

	void end() {
		timeTaken = std::chrono::duration_cast<T>(std::chrono::system_clock::now() - startTime);
	}
};

using Timer = _Timer<std::chrono::seconds>;
using SecondsTimer = _Timer<std::chrono::seconds>;
using MillisTimer = _Timer<std::chrono::milliseconds>;

template<class T>
struct _ScopedTimer : public T {
	std::string name;
	_ScopedTimer() = delete; // ScopedTimers must have names
	_ScopedTimer(std::string n) {
		name = n;
		T::start();
	}

	~_ScopedTimer() {
		T::end();
		std::cout << name << " took " << T::timeTaken.count() << "ms" << std::endl;
	}
};

using ScopedTimer = _ScopedTimer<MillisTimer>;
using ScopedSecondsTimer = _ScopedTimer<SecondsTimer>;


}
