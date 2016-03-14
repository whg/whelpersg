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

#ifndef PI
#define PI 3.141592653589793238462
#endif

template <typename T>
class Window {
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
	
	void setWindowFunc(typename Window<T>::FuncType func) { mWindowFunc = func; }

protected:
	 typename Window<T>::FuncType mWindowFunc;
};

#ifdef USE_FFTW

#include "fftw3.h"

class RealFFT  : WindowMixin<float> {
	size_t mSize;
	fftwf_complex *mOutput;
	fftwf_plan mPlan;
	float *mInput;

public:

	RealFFT(size_t size): mSize(size) {
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
	
	
	size_t getSize() { return mSize; }
	
	
	void forward(const float *input) {
		std::copy(input, input + mSize * sizeof(float), mInput);
		
		if (mWindowFunc) {
			mWindowFunc(mInput, mSize);
		}
		
		fftwf_execute(mPlan);
	}
	
	void forward(const std::vector<float> &input) {
		forward(input.data());
	}
	
protected:
	void init() {
		mInput = (float*) fftwf_malloc(sizeof(float) * mSize);
		mOutput = (fftwf_complex*) fftwf_malloc(sizeof(fftw_complex) * mSize);
		mPlan = fftwf_plan_dft_r2c_1d(static_cast<int>(mSize), mInput, mOutput, FFTW_ESTIMATE);
	}
	
	void destroy() {
		fftwf_destroy_plan(mPlan);
		fftwf_free(mInput);
		fftwf_free(mOutput);
	}
};


#endif // end USE_FFTW