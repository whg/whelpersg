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


} // end namespace audio

