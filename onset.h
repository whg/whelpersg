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
	const long SLEEP_TICKS = 12;

public:
	OnsetDetector(): mSleepCounter(0), mAverageDiff(0), mLastValue(0),
					 mCurrentState(IDLE), mGainThreshold(0.25) {
		
		mHistory.setCapacity(64);
	}
	
	bool update(T value) {
		
		T diff = value - mLastValue;
		mLastValue = value;
		mCurrentState = false;
		
//		if (diff < 0 && mState == ARMED) {
//			mCurrentState = true;
//			mState = IDLE;
//		}
		
		if (value > mGainThreshold && diff > mAverageDiff * DIFF_MARGIN && mSleepCounter <= 0) {
			mCurrentState = true;
			mSleepCounter = SLEEP_TICKS;
//			mState = ARMED;
		}
		
		if (diff > 0 && mState == IDLE) {
			
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
	
	enum State {
		IDLE, ARMED,
	};
	State mState;
};


template<typename T, int sampleRate=44100>
class TempoEstimator {
public:

	using bpmType = float;

	TempoEstimator(): mHopSize(512), mCurrentBpm(120) {
		mFFT = std::unique_ptr<dsp::RealFFT>(new dsp::RealFFT(512));
		
		mHistory.setCapacity(512);
		mIntervalCounter.setCapacity(32);
		mSecondLevelFilter.setCapacity(24);
	}
	
	float update(T value) {
		
		using namespace std;
		
		mHistory.push(value);
		
		mFFT->forward(mHistory.data().begin(), mHistory.data().end());
		mFFT->inverse(mFFT->getPower());
		const auto &ac = mFFT->getInput();
		
//		using ::operator<<;
//		auto q = vector<T>(mHistory.data().begin(),mHistory.data().end());
//		cout << q << endl;
		
		auto sorted = whg::argsort(ac.begin()+1, ac.end());
		for (int i = sorted.size()-6; i < sorted.size(); i++) {
//			std::cout << sorted[i] << "(" << binToBpm(sorted[i]+1) << ")" << ", ";
			auto rbpm = (binToBpm(sorted[i]+1));
			if (rbpm > 60 && rbpm < 180) {
				
				mIntervalCounter.increment(smoothBpm(rbpm));
			}
		}
//		std::cout << std::endl;
//

//		float bpm;
//		for (int i = sorted.size() - 1; i >= 0; i--) {
//			bpm = binToBpm(sorted[i]+1);
//			if (bpm > 60 && bpm < 165) {
//				break;
//			}
//		}
//		std::cout << bpm << std::endl;
		
		// +1 because the DC is of no use to us
//		auto maxBin = whg::argmax(ac.begin()+1, ac.end()) + 1;
//		cout << "- " << maxBin << endl;
//		float hopTime = mHopSize / static_cast<float>(sampleRate);
//		auto rbpm = reasonableBpm(binToBpm(maxBin));
//		cout << bpm << endl;
//		mIntervalCounter.increment(rbpm);
		mCurrentBpm = mIntervalCounter.getMaxKey();
//		std::cout << mIntervalCounter.data() << std::endl;
//		std::cout << modeBpm << std::endl;
		
//		mSecondLevelFilter.increment(modeBpm);
//		mBpmCounter.increment(rbpm);
//		
//		std::cout << mBpmCounter.data() << std::endl;
		
		return mCurrentBpm; //mIntervalCounter.getMaxKey();
//		std::cout << maxBin << std::endl;
//		return 60.0f / ((mHopSize / static_cast<float>(sampleRate)) * modeBin);
	}
	
	void setHopSize(size_t hs) { mHopSize = hs; }
	size_t getHopSize() { return mHopSize; }
	
protected:
	size_t mHopSize;
	std::unique_ptr<dsp::RealFFT> mFFT;
//	std::deque<T> mHistory;
	whg::HistoryQueue<T> mHistory;
	whg::IntervalCounter<float> mIntervalCounter;
	whg::IntervalCounter<float> mSecondLevelFilter;
	whg::Counter<float> mBpmCounter;
	
	bpmType mCurrentBpm;
	
	bpmType binToBpm(size_t binNum) const {

		return 60.0f / ((mHopSize / static_cast<float>(sampleRate)) * binNum);
	}
	
	bpmType reasonableBpm(bpmType bpm) const {

		if (bpm < 0 || bpm > 1e6) return 0;
	
		while (bpm < 60) {
			bpm*= 2;
		}
		
		while (bpm > 164) {
			bpm/= 2;
		}
		return bpm;
	}
	
	bpmType smoothBpm(bpmType bpm) {
		bpmType doubleBpm = bpm * 2;
		bpmType halfBpm = bpm * 0.5;
		static const bpmType margin = 1.5;
		if (doubleBpm < mCurrentBpm + margin && doubleBpm > mCurrentBpm - margin) {
			return mCurrentBpm;
		}
//		else if (halfBpm < mCurrentBpm + margin && halfBpm > mCurrentBpm - margin) {
//			return bpm;
//		}
		return bpm;
	}
	
};

}