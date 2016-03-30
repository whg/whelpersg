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
public:
	OnsetDetector(): mHistoryMaxLength(64), mTopThreshold(0.65), mSleepCounter(0),
	mBottomTheshold(0.9), mAverageDiff(0), mState(IDLE) {
		
		mHistory.setCapacity(mHistoryMaxLength);
	}
	
	
	bool update(T value) {
		
		T diff = value - mLastValue;
		mLastValue = value;
		mCurrentState = false;
		if (diff > mAverageDiff * 2 && mSleepCounter <= 0) {
			//			return true;
			mCurrentState = true;
			mSleepCounter = 10;
			//			std::cout << diff << " : " << mAverageDiff << std::endl;
			
		}
		if (diff > 0) {
			//		else {
			
			mHistory.push(diff);
			
//			mHistory.push_back(diff);
//			
//			while (mHistory.size() > mHistoryMaxLength) {
//				mHistory.pop_front();
//			}
			
//			T sum = 0;
//			for (auto &v : mHistory) {
//				sum+= v;
//			}

			mAverageDiff = whg::mean(mHistory.data());// sum / mHistory.size();
		}
		
		mSleepCounter--;
		return mCurrentState;
		
		//
		//		mHistory.push_back(value);
		//
		//		while (mHistory.size() > mHistoryMaxLength) {
		//			mHistory.pop_front();
		//		}
		//
		//		// TODO: don't go over whole history on every update
		//		// keep track of position of max and update if get's popped
		//		mMaxValue = 0;
		//		for (auto &v : mHistory) {
		//			mMaxValue = std::max(v, mMaxValue);
		//		}
		//
		//		std::vector<T> normalisedValues(mHistory.size());
		//		for (size_t i = 0; i < mHistory.size(); i++) {
		////			v/= mMaxValue;
		//			normalisedValues[i] = mHistory[i] / mMaxValue;
		//		}
		//
		//		auto normalisedValue = mHistory.back();// / mMaxValue;
		//		if (normalisedValue >= mTopThreshold) {
		//			mState = TOPPED;
		//		}
		////		else if (normalinormalisedValue < mBottomTheshold && mState == TOPPED) {
		//		else if (normalisedValue < mLastValue && mState == TOPPED) {
		//			mState = BOTTOMED;
		//		}
		//		else {
		//			mState = IDLE;
		//		}
		//
		//		mLastValue = normalisedValue;
		////		std::cout << value << " : " << normalisedValue << " : " << mMaxValue << std::endl;
		//		return mState == BOTTOMED;
	}
	
	bool getCurrentState() const { return mCurrentState; }
	
public:
//	std::deque<float> mHistory;
	whg::HistoryQueue<float> mHistory;
	size_t mHistoryMaxLength;
	
	T mMaxValue, mLastValue;
	T mTopThreshold, mBottomTheshold;
	T mAverageDiff;
	long mSleepCounter;
	bool mCurrentState;
	
	enum State {
		IDLE, TOPPED, BOTTOMED
	};
	
	State mState;
};


template<typename T, size_t SIZE=256>
class TempoEstimator {
public:

	TempoEstimator() {
		mFFT = std::unique_ptr<dsp::RealFFT>(new dsp::RealFFT(SIZE));
	}
	
	T update(T value) {
		
		mHistory.push_back(value);
		while (mHistory.size() > SIZE) {
			mHistory.pop_front();
		}
		
		
		mFFT->forward(mHistory.begin(), mHistory.end());
		mFFT->inverse(mFFT->getPower());
		const auto &ac = mFFT->getInput();
		
//		auto mac = std::max_element(ac.begin(), ac.end());
		
		auto mac = whg::argmax(ac.begin()+1, ac.end());
		return mac;
	}
	
	void setHopSize(size_t hs) { mHopSize = hs; }
	size_t getHopSize() { return mHopSize; }
	
protected:
	size_t mHopSize;
	std::unique_ptr<dsp::RealFFT> mFFT;
	std::deque<T> mHistory;
	
};

}