#pragma once

#include "whelpersg/dsp.h"
#include "whelpersg/audio.h"
#include "whelpersg/math.h"
#include "whelpersg/util.h"
#include "whelpersg/data.h"

#include <algorithm>

namespace whg {

template<typename T>
class OnsetDetector {
	
	const float DIFF_MARGIN = 2.0f;
	const long SLEEP_TICKS = 24;

public:
	OnsetDetector(): mSleepCounter(0), mAverageDiff(0), mLastValue(0), mGainThreshold(0.25) {
		
		mHistory.setCapacity(64);
	}
	
	bool update(T value) {
		
		T diff = value - mLastValue;
		mLastValue = value;
		mCurrentState = false;

		if (value > mGainThreshold && diff > mAverageDiff * DIFF_MARGIN && mSleepCounter <= 0) {
			mCurrentState = true;
			mSleepCounter = SLEEP_TICKS;
		}
		
		// effectively half wave rectifying
		if (diff > 0) {
			
			mHistory.push(diff);
			mAverageDiff = whg::mean(mHistory.data());
		}
		
		mSleepCounter--;
		return mCurrentState;

	}
	
	bool getCurrentState() const { return mCurrentState; }
	
protected:
	whg::HistoryQueue<float> mHistory;
	
	T mAverageDiff, mLastValue;
	long mSleepCounter;
	bool mCurrentState;
	T mGainThreshold;
	
};


template<typename T, int sampleRate=44100>
class TempoEstimator {
public:

	using bpmType = float;
	
	TempoEstimator(): mHopSize(512), mCurrentBpm(120), mFFTPower(512, 0) {
		mFFT = std::unique_ptr<dsp::RealFFT>(new dsp::RealFFT(512));
		
		mHistory.setCapacity(512);
		mIntervalCounter.setCapacity(32);
	}
	
	float update(T value) {
		
		mHistory.push(value);
		
		if (mHistory.data().size() != 512) return 0;
		
		// autocorrelation via FFT
		mFFT->forward(mHistory.data().begin(), mHistory.data().end());
		mFFT->getPower(mFFTPower);
		mFFT->inverse(mFFTPower);
		const auto &ac = mFFT->getInput();

		// + 1 as we never will want the DC component
		auto sorted = whg::argsort(ac.begin()+1, ac.end());
        for (size_t i = sorted.size(); --i > sorted.size() - 6; ) {
			auto rbpm = (binToBpm(sorted[i]+1));
			if (rbpm > 69 && rbpm < 180) {
				
				mIntervalCounter.increment(smoothBpm(rbpm));
                mFoundValue = ac[sorted[i]+1];
                break;
			}
		}

//        auto localmaxes = whg::localmax(ac.begin(), ac.end());
//		auto sorted = whg::argsort(ac.begin(), ac.end());
//        for (size_t i = sorted.size(); --i > 0; ) {
//            size_t index = sorted[i];
//            if (localmaxes[index]) {
//                std::vector<float> mults = { 1.0f / 3.0f, 0.5f, 1.0f, 2.0f, 3.0f };
//                float maxval = 0.0f;
//                size_t maxbin = 0;
//                for (auto mult : mults) {
//                    size_t bin = static_cast<size_t>(index * mult);
//                    if (bin < 511) {
//                        if (ac[bin] > maxval) {
//                            maxbin = bin;
//                            maxval = ac[bin];
//                        }
//                    }
//                }
//                auto bpm = binToBpm(maxbin);
//                if (bpm > 69 && bpm < 180) {
//                    mIntervalCounter.increment(smoothBpm(bpm));
//                }
//            }
//        }

		mCurrentBpm = mIntervalCounter.getMaxKey();
		
		return mCurrentBpm;
	}
	
	void setHopSize(size_t hs) { mHopSize = hs; }
	
	size_t getHopSize() { return mHopSize; }
	
protected:
	size_t mHopSize;
	std::unique_ptr<dsp::RealFFT> mFFT;
	whg::HistoryQueue<T> mHistory;
	whg::IntervalCounter<float> mIntervalCounter;
	std::vector<float> mFFTPower;
	
	bpmType mCurrentBpm;
	
	bpmType binToBpm(size_t binNum) const {

		return 60.0f / (mHopSize / static_cast<float>(sampleRate) * binNum);
	}
	
	bpmType smoothBpm(bpmType bpm) {
		bpmType doubleBpm = bpm * 2;
		static const bpmType margin = 1.5;
		if (doubleBpm < mCurrentBpm + margin && doubleBpm > mCurrentBpm - margin) {
			return mCurrentBpm;
		}
		return bpm;
	}
    
public:
    float mFoundValue;
	
};

}