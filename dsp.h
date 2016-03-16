//
//  dsp.h
//  hlaa
//
//  Created by Will Gallia on 14/03/2016.
//  Copyright (c) 2016 None. All rights reserved.
//

#pragma once

#include <algorithm>
#include <vector>
#include <cmath>
#include <complex>

#ifndef PI
#define PI 3.141592653589793238462
#endif

template <typename T>
class window {
public:
	using FuncType = std::function<void(T *, size_t)>;
	
	static FuncType createBartlett() {
		return [](T * input, size_t N) {
			for (size_t i = 0; i < N; i++) {
				input[i]*= 1.0 - std::abs(2.0 * i / (N - 1.0) - 1.0);

			}
		};
	}
	
	static FuncType createWelch() {
		return [](T * input, size_t N) {
			
			static std::vector<T> lookup;
			if (lookup.size() != N) {
				lookup.resize(N);
				T nm1o2 = (N - 1.0) / 2.0, temp;
				for (size_t i = 0; i < N; i++) {
					temp = (i - nm1o2) / nm1o2;
					lookup[i] = 1.0 - temp * temp;
				}
			}
		
			for (size_t i = 0; i < N; i++) {
				input[i]*= lookup[i];
			}
		};
	}
	
	static FuncType createHann() {
		return [](T * input, size_t N) {
			
			static std::vector<T> lookup;
			if (lookup.size() != N) {
				lookup.resize(N);
				for (size_t i = 0; i < N; i++) {
					lookup[i] = 0.5 * (1.0 - std::cos(2.0 * PI * i / (N - 1.0)));
				}
			}
			
			for (size_t i = 0; i < N; i++) {
				input[i]*= lookup[i];
			}
		};
	}
	
	static FuncType createBlackman(float alpha=0.16) {
		return [alpha](T * input, size_t N) {
			
			static std::vector<T> lookup;
			if (lookup.size() != N) {
				lookup.resize(N);
				double a0 = (1.0-alpha)/2.0, a1 = 0.5, a2 = alpha / 2.0;
				double theta;
				for (size_t i = 0; i < N; i++) {
					theta = 2 * PI * i / (N - 1);
					lookup[i] = a0 - a1 * std::cos(theta) + a2 * std::cos(theta* 2.0);
				}
			}
			
			for (size_t i = 0; i < N; i++) {
				input[i]*= lookup[i];
			}
		};
	}

};



template <typename T>
class WindowMixin {
	
public:
	WindowMixin(): mWindowFunc(nullptr) {}
	
	void setWindowFunc(typename window<T>::FuncType func) { mWindowFunc = func; }

protected:
	 typename window<T>::FuncType mWindowFunc;
};

template <typename T>
class BaseFFT {
public:
	BaseFFT(size_t size): mSize(size) {}
	
	size_t getSize() { return mSize; }

public:
	size_t mSize;
	std::vector<std::complex<T>> mOutput;
	std::vector<T> mInput;
};

#ifdef USE_FFTW

#include "fftw3.h"

class RealFFT : public BaseFFT<float>, public WindowMixin<float> {
public:
	fftwf_plan mPlan;
	bool mNormalisesOutput;
	
	
public:

	RealFFT(size_t size): mNormalisesOutput(true), BaseFFT(size) {
		init();
	}
	
	~RealFFT() {
		destroy();
	}
	
	void resize(size_t size) {
		mSize = size;
		destroy();
		init();
	}
	
	
	void forward(const std::vector<float> &input) {
		
		std::copy(input.begin(), input.end(), mInput.begin());
		
		if (mWindowFunc) {
			mWindowFunc(&mInput[0], mSize);
		}
		
		fftwf_execute(mPlan);
		
		if (mNormalisesOutput) {
			float size = static_cast<float>(mSize);
			for (size_t i = 0; i < mSize; i++) {
				mOutput[i]  /= size;
			}
		}
		
	}
	
	double frequencyForBin(uint bin, uint sampleRate=44100) const {
		return bin / static_cast<double>(mSize) * sampleRate;
	}
	
	
	void setNormalisesOutput(bool b) { mNormalisesOutput = b; }
	
	bool getNormalisesOutput() const { return mNormalisesOutput; }
	
protected:
	void init() {
		mInput.resize(mSize);
		mOutput.resize(mSize);
		
		// the storage of fftwf_complex is compatible with std::complex
		mPlan = fftwf_plan_dft_r2c_1d(static_cast<int>(mSize),
									  static_cast<float*>(&mInput[0]),
									  reinterpret_cast<fftwf_complex*>(&mOutput[0]),
									  FFTW_ESTIMATE);
	}
	
	void destroy() {
		fftwf_destroy_plan(mPlan);
	}
};


#endif // end USE_FFTW


struct TransformSettings {
	uint nbins, sampleRate, size;
};

struct ChromaFilterSettings : public TransformSettings {
	uint chroma, numChromas;
};


inline double frequencyForBin(uint bin, uint size, uint sampleRate=44100) {
	return bin / static_cast<double>(size) * sampleRate;
}

/// frequencies for real part of the spectrum
template <typename T>
std::vector<T> fftFrequencies(TransformSettings s) {
	std::vector<T> output(s.nbins);
	for (uint i = 0; i < s.nbins; i++) {
		output[i] = i / static_cast<T>(s.size) * s.sampleRate;
	}
	return output;
}


#include "whelpersg/audio.h"


template <typename T>
inline void chromaFilter(ChromaFilterSettings s, std::vector<T> &output) {

	output.resize(s.nbins);
	
	std::vector<T> chromaNumbers(s.nbins), binWidths(s.nbins), loudnessAdjustment(s.nbins);
	
	const T halfNumChromas = std::round(s.numChromas / 2.0);
	const double gaussianSkinnines = 2.0;
	const T minBinWidth = 1.0;
	
	T frequency, lastNote = 0, chromaNumber, temp;
	
	for (uint i = 1; i < s.nbins; i++) {
	
		// bin frequency
		frequency = static_cast<T>(frequencyForBin(i, s.nbins, s.sampleRate));
		
		// when chromas = 12, this is like audio::ftom() (just offset)
		chromaNumber = audio::ftoo(frequency) * s.numChromas;
		
		// modulo so we wrap around numChromas and subtract half to centre on 0
		// this will give us part of the exponent later
		chromaNumbers[i] =  std::fmod(chromaNumber - s.chroma + halfNumChromas, s.numChromas) - halfNumChromas;
		
		// calculate the loudness ramp values
		// TODO: make this better!
		temp = (chromaNumber / s.numChromas - 5.0) / 2.0;
		loudnessAdjustment[i] = std::exp(-0.5 * temp * temp);
		
		
		binWidths[i-1] = std::max(chromaNumber - lastNote, minBinWidth);
		lastNote = chromaNumber;
	}

	binWidths[s.nbins-1] = minBinWidth;
	
	for (uint i = 1; i < s.nbins; i++) {
		temp = gaussianSkinnines * chromaNumbers[i] / binWidths[i];
		output[i] = std::exp(-0.5 * temp * temp) * loudnessAdjustment[i];
		
	}
	
	output[0] = 0; // always nothing on the DC
	
}

struct MelScaleSettings {
	double minFrequency, maxFrequency;
	uint numBands;
};

struct MelFilterSettings : public MelScaleSettings, public TransformSettings {};

template <typename T>
inline std::vector<T> melFrequencies(MelScaleSettings s) {
	
	using namespace audio;
	
	double lowerMelBand = ftomel(s.minFrequency);
	double step = (ftomel(s.maxFrequency) - lowerMelBand) / (s.numBands - 1);
	// - 1 so we reach the max frequency in numBands
	
	std::vector<T> frequencies(s.numBands);
	
	for (uint i = 0; i < s.numBands; i++) {
		frequencies[i] = meltof(lowerMelBand + step * i);
	}
	
	return frequencies;
}


template <typename T>
inline void melFilter(std::vector<std::vector<T>> &output, MelFilterSettings s) {
	
	MelScaleSettings scaleSettings = s;
	scaleSettings.numBands+= 2;
	auto melFreqs(melFrequencies<T>(scaleSettings));
	
	output.resize(s.numBands, std::vector<T>(s.nbins, 0.0));
	
	auto fftFreqs(fftFrequencies<T>(s));
	
	for (uint band = 0; band < s.numBands; band++) {
		
		double lowerFreq = melFreqs[band];
		double middleFreq = melFreqs[band+1];
		double upperFreq = melFreqs[band + 2];

		uint lowerBin = lowerFreq * s.size / s.sampleRate;
		uint upperBin = upperFreq * s.size / s.sampleRate;
		double binFreq, risingGradient, fallingGradient;
		
		for (uint bin = lowerBin; bin < upperBin; bin++) {
			binFreq = fftFreqs[bin]; //fftFreqbin / static_cast<double>(s.nbins) * s.sampleRate;
			
			risingGradient = (binFreq - lowerFreq) / (middleFreq - lowerFreq);
			fallingGradient = (upperFreq - binFreq) / (upperFreq - middleFreq);
			
			output[band][bin] = std::max(0.0, std::min(risingGradient, fallingGradient));
		}
	}
	
}
