#pragma once

#include <algorithm>
#include <vector>
#include <cmath>
#include <complex>
#include <type_traits>

#include "whelpersg/audio.h"


#ifndef PI
#define PI 3.141592653589793238462
#endif


namespace dsp {

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



#ifdef USE_FFTW

#include "fftw3.h"


// super basic, just encapsulate fftw's memory management
template<typename T>
class fftwfAllocator {
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;
	
	pointer address(reference r) { return &r; }
	const_pointer address(const_reference r) { return &r; }
	
	pointer allocate(size_type N, typename std::allocator<void>::const_pointer=0) {

		return reinterpret_cast<pointer>(fftwf_malloc(N * sizeof(T)));
	}
	
	void deallocate(pointer p, size_type) {
	
		fftwf_free(static_cast<void*>(p));
	}
	
	size_type max_size() const {
		return std::numeric_limits<size_type>::max() / sizeof(T);
	}
	
	void construct(pointer p, const T& t) { new(p) T(t); }
	void destroy(pointer p) { p->~T(); }

};


template <typename T>
class BaseFFT {
public:
	
	using inputType = std::vector<T>;
	using outputType = std::vector<std::complex<T>>;
	
	BaseFFT(size_t size): mSize(size) {}
	
	size_t getSize() const { return mSize; }
	
	void getPower(std::vector<T> &output) const {
		
		const size_t N = mSize / 2 + 1;
		output.resize(N);
		
		for (size_t i = 0; i < N; i++) {
			output[i] = std::abs(mOutput[i]);
		}
	}
	
	std::vector<T> getPower() const {
		
		const size_t N = mSize / 2 + 1;
		std::vector<T> output(N);
		
		for (size_t i = 0; i < N; i++) {
			output[i] = std::abs(mOutput[i]);
		}
		return output;
	}

	
	const outputType& getOutput() const { return mOutput; }
	outputType& getOutput() { return mOutput; }
	
	const inputType& getInput() const { return mInput; }
	inputType& getInput() { return mInput; }
	
public:
	size_t mSize;
	outputType mOutput;
	inputType mInput;
};

#define FFTW_COPY_OUTPUT 1


class RealFFT : public BaseFFT<float>, public WindowMixin<float> {
public:
	fftwf_plan mForwardPlan, mInversePlan;
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
	
	template<class InputIterator>
	void forward(InputIterator begin, InputIterator end) {
	
		mInput.assign(begin, end);
		forwardExecute();
	}
	
//	void forward(inputType::const_iterator begin, inputType::const_iterator end) {
//		
//		mInput.assign(begin, end);
//		forwardExecute();
//	}
	
	void forward(const inputType &input) {
		
		forward(input.begin(), input.end());
	}
	
	void forward(const float *input) {
		std::copy(input, input + mSize, mInput.begin());
		forwardExecute();
	}
	
	void inverse(const outputType &input) {
		
		mOutput.assign(input.begin(), input.end());
		//		std::copy(input.begin(), input.end(), mInput.begin());
		inverseExecute();
	}
	
	// when the input is a vector of real numbers
	void inverse(const inputType &input) {
		
		for (size_t i = 0; i < input.size(); i++) {
			mOutput[i].real(input[i]);
		}
		
		inverseExecute();
	}
	
	double frequencyForBin(uint bin, uint sampleRate=44100) const {
		return bin / static_cast<double>(mSize) * sampleRate;
	}
	
	
	void setNormalisesOutput(bool b) { mNormalisesOutput = b; }
	
	bool getNormalisesOutput() const { return mNormalisesOutput; }
	
	
protected:
#ifdef FFTW_COPY_OUTPUT
	fftwf_complex *tOutput;
#endif

	void init() {
		mInput.resize(mSize);
		auto outputSize = mSize / 2 + 1;
		mOutput.resize(outputSize);

#ifdef FFTW_COPY_OUTPUT
		tOutput = (fftwf_complex*) fftwf_malloc(outputSize * sizeof(fftwf_complex));
#endif
		
		mForwardPlan = fftwf_plan_dft_r2c_1d(static_cast<int>(mSize),
									  static_cast<float*>(&mInput[0]),
#ifdef FFTW_COPY_OUTPUT
									  tOutput,
#else
									  reinterpret_cast<fftwf_complex*>(&mOutput[0]),
#endif
									  FFTW_ESTIMATE);
		
		mInversePlan = fftwf_plan_dft_c2r_1d(static_cast<int>(mSize),
#ifdef FFTW_COPY_OUTPUT
											 tOutput,
#else
											 reinterpret_cast<fftwf_complex*>(&mOutput[0]),
#endif
											static_cast<float*>(&mInput[0]),
											 FFTW_ESTIMATE);

	}
	
	void destroy() {
#ifdef FFTW_COPY_OUTPUT
		fftwf_free(tOutput);
#endif

		fftwf_destroy_plan(mForwardPlan);
		fftwf_destroy_plan(mInversePlan);
		
	}
	
	void forwardExecute() {
		if (mWindowFunc) {
			mWindowFunc(&mInput[0], mSize);
		}
		
		fftwf_execute(mForwardPlan);
		
#ifdef FFTW_COPY_OUTPUT
		std::complex<float> *tempOutput = reinterpret_cast<std::complex<float>*>(tOutput);
		std::copy(tempOutput, tempOutput + mSize / 2 + 1, mOutput.begin());
#endif
		
		if (mNormalisesOutput) {
			float size = static_cast<float>(mSize);
			for (size_t i = 0; i < mSize; i++) {
				mOutput[i]  /= size;
			}
		}
	}
	
	void inverseExecute() {

		if (!mNormalisesOutput) {
			float size = static_cast<float>(mSize);
			for (size_t i = 0; i < mSize; i++) {
				mOutput[i]  /= size;
			}
		}


#ifdef FFTW_COPY_OUTPUT
		std::complex<float> *tempOutput = reinterpret_cast<std::complex<float>*>(tOutput);
		std::copy(mOutput.begin(), mOutput.end(), tempOutput);
#endif

		
		fftwf_execute(mInversePlan);
		
		
	}
};


#endif // end USE_FFTW


struct TransformSettings {
	uint nbins, sampleRate, size;
	
	void setSize(uint s) {
		size = s;
		nbins = size / 2 + 1;
	}
};

struct ChromaFilterSettings : public TransformSettings {
	uint numChromas;
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

template <typename T>
inline std::vector<std::vector<T>> chromaFilterbank(ChromaFilterSettings s) {
	
	std::vector<std::vector<T>> output(s.numChromas, std::vector<T>(s.nbins, 0.0));

	
	std::vector<T> chromaNumbers(s.nbins), binWidths(s.nbins), loudnessAdjustment(s.nbins);
	
	const T halfNumChromas = std::round(s.numChromas / 2.0);
	const double gaussianSkinnines = 4.0;
	const T minBinWidth = 1.0;
	
	T frequency, lastNote = 0, chromaNumber, temp;

	for (uint chroma = 0; chroma < s.numChromas; chroma++) {
		
		auto &band = output[chroma];
		
		for (uint i = 1; i < s.nbins; i++) {
		
			// bin frequency
			frequency = static_cast<T>(frequencyForBin(i, s.nbins, s.sampleRate));
			
			// when chromas = 12, this is like audio::ftom() (just offset)
			chromaNumber = audio::ftoo(frequency) * s.numChromas;
			
			// modulo so we wrap around numChromas and subtract half to centre on 0
			// this will give us part of the exponent later
			chromaNumbers[i] =  std::fmod(chromaNumber - chroma + halfNumChromas, s.numChromas) - halfNumChromas;
			
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
			band[i] = std::exp(-0.5 * temp * temp) * loudnessAdjustment[i];
			
		}
		band[0] = 0; // always nothing on the DC
		
	}
	
	return output;
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
inline std::vector<std::vector<T>> melFilterbank(MelFilterSettings s) {
	
	MelScaleSettings scaleSettings = s;
	scaleSettings.numBands+= 2;
	auto melFreqs(melFrequencies<T>(scaleSettings));
	
	std::vector<std::vector<T>> output(s.numBands, std::vector<T>(s.nbins, 0.0));
	
	auto fftFreqs(fftFrequencies<T>(s));
	
	for (uint band = 0; band < s.numBands; band++) {
		
		double lowerFreq = melFreqs[band];
		double middleFreq = melFreqs[band + 1];
		double upperFreq = melFreqs[band + 2];

		uint lowerBin = lowerFreq * s.size / s.sampleRate;
		uint upperBin = upperFreq * s.size / s.sampleRate;
		double binFreq, risingGradient, fallingGradient;
		
		T v, sum = 0;
		for (uint bin = lowerBin; bin < upperBin; bin++) {
			binFreq = fftFreqs[bin];
			
			risingGradient = (binFreq - lowerFreq) / (middleFreq - lowerFreq);
			fallingGradient = (upperFreq - binFreq) / (upperFreq - middleFreq);
			
			v = std::max(0.0, std::min(risingGradient, fallingGradient));
			output[band][bin] = v;
			sum+= v;
		}
		
//		for (uint bin = lowerBin; bin < upperBin; bin++) {
//			output[band][bin] /= sum;
//		}
	}
	
	return output;
}

template<typename T>
std::vector<T> autocorrelate(const std::vector<T> &input) {
	RealFFT fft(input.size());
	
	fft.forward(input);
	fft.inverse(fft.getPower());
	
	return fft.getInput();
}

} // namespace dsp