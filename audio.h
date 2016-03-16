#pragma once

#include <cmath>


namespace audio {

//const double concertPitch = 440.0;
const double C0_frequency = 32.703195662574828;

inline double mtof(double midiNote) {
	return std::pow(2.0, (midiNote - 69.0) / 12.0) * 440.0;
}

inline double ftom(double frequency) {
	return 69.0 + 12.0 * std::log2(frequency / 440.0);
}

/// floating point octave relative to MIDI (C-2)
inline double ftoo(double frequency) {
	return std::log2(frequency / C0_frequency);
}

inline double ftomel(double frequency) {
	return 1127.0 * std::log(frequency / 700.0 + 1.0);
}

inline double meltof(double mel) {
	return 700.0 * (std::exp(mel / 1127.0) - 1.0);
}



template <typename T>
inline void logAmplitude(std::vector<T> &realVector) {
	for (auto &v : realVector) {
		v = static_cast<T>(20.0 * std::log10(v));
	}
}



} // end namespace audio

