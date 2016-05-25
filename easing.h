#pragma once

#include <deque>

namespace whg {

template <typename T>
class Easer {
public:
	
	Easer(float startTime, float endTime, T start, T end):
	mStartTime(startTime), mEndTime(endTime), mCurrentValue(0),
	mStartValue(start), mEndValue(end) {
		mTimeRange = mEndTime - mStartTime;
		mValueRange = end - start;
	}
	
	Easer(float duration, T start, T end): Easer(0, duration, start, end) {}
	Easer(float duration, T value): Easer(0, duration, value, value) {}
	
	virtual T valueAt(float time) const { return mStartValue; }
	
	virtual T update(float time) {
		mCurrentValue = valueAt(time);
		return mCurrentValue;
	}
	
	virtual T getValue() const { return mCurrentValue; }
	
	bool getIsAlive(float time) const { return time >= mStartTime && time <= mEndTime; }
	bool getHasEnded(float time) const { return time > mEndTime; }
	
	float getStartTime() const { return mStartTime; }
	float getEndTime() const { return mEndTime; }
	
protected:
	T mStartValue, mEndValue, mValueRange;
	float mStartTime, mEndTime, mTimeRange;
	T mCurrentValue;
	
	template <typename U>
	friend class EasingChain;
	
};

template <typename T>
class LinearEaser : public Easer<T> {
public:
	
	LinearEaser(float duration, T start, T end): Easer<T>(duration, start, end) {}
	LinearEaser(float startTime, float endTime, T start, T end): Easer<T>(startTime, endTime, start, end) {}
	
	T valueAt(float time) const override {
	return this->mValueRange * clamp((time - this->mStartTime) / this->mTimeRange) + this->mStartValue;
}
};

// thanks to Robert Penner (& http://gizma.com/easing/)

template <typename T>
class QuadInEaser: public Easer<T> {
public:
	
	QuadInEaser(float duration, T start, T end): Easer<T>(duration, start, end) {}
	QuadInEaser(float startTime, float endTime, T start, T end): Easer<T>(startTime, endTime, start, end) {}
	
	T valueAt(float time) const override {
		float t = clamp((time - this->mStartTime) / this->mTimeRange);
		return this->mValueRange * t * t + this->mStartValue;
	}
};

template <typename T>
class QuadOutEaser: public Easer<T> {
public:
	
	QuadOutEaser(float duration, T start, T end): Easer<T>(duration, start, end) {}
	QuadOutEaser(float startTime, float endTime, T start, T end): Easer<T>(startTime, endTime, start, end) {}
	
	T valueAt(float time) const override {
		float t = clamp((time - this->mStartTime) / this->mTimeRange);
		return -this->mValueRange * t * (t - 2) + this->mStartValue;
	}
};

template <typename T>
class QuadInOutEaser: public Easer<T> {
public:
	
	QuadInOutEaser(float duration, T start, T end): Easer<T>(duration, start, end) {}
	QuadInOutEaser(float startTime, float endTime, T start, T end): Easer<T>(startTime, endTime, start, end) {}
	
	T valueAt(float time) const override {
	
		float t = clamp((time - this->mStartTime) / (this->mTimeRange / 2), 0.0f, 2.0f);
		if (t < 1) {
			return this->mValueRange / 2 * t * t + this->mStartValue;
		}
		--t;
		return -this->mValueRange / 2 * (t * (t-2) - 1) + this->mStartValue;
	}
};


template <typename T>
class EasingChain : public Easer<T> {
public:
	
	EasingChain(): Easer<T>(0, 0, 0) {}
	EasingChain(float duration, T start, T end): Easer<T>(duration, start, end) {}
	
	template <class EasingClass>
	void extend(float duration, T endValue) {
		add(new EasingClass(duration, mEasers.back()->mEndValue, endValue));
	}
	
	void add(Easer<T> *easer) {
		
		if (mEasers.size() > 0) {
			
			if (easer->mStartTime == 0) {
				easer->mStartTime = mEasers.back()->mEndTime;
				easer->mEndTime+= mEasers.back()->mEndTime;
			}
		}
		else {
			this->mStartTime = easer->mStartTime;
			this->mStartValue = easer->mStartValue;
		}
		
		this->mEndTime = easer->mEndTime;
		this->mEndValue = easer->mEndValue;
		
		mEasers.push_back(std::unique_ptr<Easer<T>>(easer));
		
	}
	
	T valueAt(float time) const {
		if (time >= this->mEndTime) {
			return this->mEndValue;
		}
		
		for (const auto &easer : mEasers) {
			if (time < easer->mEndTime) {
				return easer->valueAt(time);
			}
		}
		
		return this->mStartValue;
	}
	
	T update(float time) {
		
		if (time >= this->mEndTime) {
			mEasers.clear();
			this->mCurrentValue = this->mEndValue;
		}
		else {
			
			size_t i = 0;
			for (const auto &easer : mEasers) {
				if (time < easer->mEndTime) {
					this->mCurrentValue = easer->valueAt(time);
					break;
				}
				i++;
			}

			while (i > 0) {
				mEasers.pop_front();
				i--;
			}
		}
	
		return this->mCurrentValue;
	}
	
	void clear() {
		mEasers.clear();
		this->mStartTime = this->mEndTime = 0;
	}
	
	bool empty() const {
		return mEasers.empty();
	}
	
	size_t size() const {
		return mEasers.size();
	}
	
protected:
	std::deque<std::unique_ptr<Easer<T>>> mEasers;
};


} // namespace whg