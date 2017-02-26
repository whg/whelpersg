#pragma once

#include <vector>
#include <deque>
#include <unordered_map>
#include <iterator>
#include <algorithm>
#include <numeric>

template<typename T>
inline std::ostream& operator<<(std::ostream &os, const std::vector<T> &vec) {
	os << "[";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        os << *it << (std::next(it) == vec.end() ? "" : ", ");
	}
	return os << "]";
}

template<typename K, typename V>
inline std::ostream& operator<<(std::ostream &os, const std::unordered_map<K, V> &map) {
	os << "{";
	for (auto it = map.begin(); it != map.end(); ++it) {
		os << it->first << ": " << it->second << (std::next(it) == map.end() ? "" : ", ");
	}
	return os << "}";
}

namespace whg {

template <typename T>
class HistoryQueue {
	
public:
	
	void push(T value) {
		mData.push_back(value);
		while (mData.size() > mCapacity) {
			mData.pop_front();
		}
	}
	
	const std::deque<T>& data() const { return mData; }
	
	void setCapacity(size_t c) { mCapacity = c; }
	
	size_t getCapacity() { return mCapacity; }
	
protected:
	std::deque<T> mData;
	size_t mCapacity;
};

template <typename T>
class Counter {
public:
	
	using dataType = std::unordered_map<T, uint>;
	
	virtual uint increment(T value) {
		
		auto it = mMap.find(value);
		
		if (it == mMap.end()) {
			mMap.emplace(value, 1);
			return 1;
		}
		else {
			return ++(*it).second;
		}
		
	}
	
	virtual uint decrement(T value) {
		
		auto it = mMap.find(value);
		if (it != mMap.end()) {
			auto count = --(*it).second;

			if (count == 0) {
				mMap.erase(it);
			}
			return count;
		}
		return 0;
	}
	
	T getMaxKey() const {
		uint max = 0;
		T output = T();
		for (const auto &pair : mMap) {
			if (pair.second > max) {
				output = pair.first;
				max = pair.second;
			}
		}
		return output;
	}
	
	const dataType& data() const { return mMap; }
	
	void setCapacity(size_t c) { mCapacity = c; }
	
	size_t getCapacity() { return mCapacity; }
	
protected:
	dataType mMap;
	size_t mCapacity;
};

template <typename T>
class IntervalCounter : public Counter<T> {
public:

	IntervalCounter(size_t size=64) : mIntervalSize(size) {}

	virtual uint increment(T value) override {
	
		uint output = Counter<T>::increment(value);
	
		mHistory.push_back(value);
		if (mHistory.size() > mIntervalSize) {
			Counter<T>::decrement(mHistory.front());
			mHistory.pop_front();
		}
	
		return output;
	}
	
	void setIntervalSize(size_t s) { mIntervalSize = s; }
	size_t getIntervalSize() { return mIntervalSize; }
	
protected:
	std::deque<T> mHistory;
	size_t mIntervalSize;
};


template<typename T>
struct KalmanFilter1D {
    
    KalmanFilter1D(): q(1), r(1), p(1) {}
    
    KalmanFilter1D(T q, T r, T inital):
    q(q), r(r), p(1), x(inital) {}
    
    T update(T measurement) {
        p+= q;
        
        k = p / (p + r);
        x = x + k * (measurement - x);
        p = (1 - k) * p;
        
//        printf("%f,%f,%f,%f,%f\n", p, q, k, x, r);
        return x;
    }
    
    T q; //process noise covariance
    T r; //measurement noise covariance
    T x; //value
    T p; //estimation error covariance
    T k; //kalman gain
};

template <typename T>
struct DecreasingValue {
    
    DecreasingValue(): mChangeRate(1) {}
    DecreasingValue(float r): mChangeRate(r) {}
    
    virtual T update(float currentTimef, T value) {
        if (value >= mValue) {
            mValue = value;
        }
        else {
//            mValue-= mChangeRate * currentTimef;
        }
        this->mLastUpdate = currentTimef;

        return mValue;
    }
    
    void setChangeRate(float r) { mChangeRate = r; }
    
    T mValue;
    float mChangeRate;
    float mLastUpdate;
};

template <typename T>
struct ContantChangeValue : public DecreasingValue<T> {
    
    ContantChangeValue(float r): DecreasingValue<T>(r) {}

    
    T update(float currentTimef, T value) override {
        float change = this->mChangeRate * (currentTimef - this->mLastUpdate);
        if (value > this->mValue + change) {
            this->mValue+= change;
        }
        else if (value < this->mValue - change) {
            this->mValue-= change;
        }
    
        this->mLastUpdate = currentTimef;
        return this->mValue;
    }
};

template <typename T>
struct MovingAverage {
    
    MovingAverage( size_t length ): mHistoryLength( length ) {}
    
    void setHistoryLength( size_t length ) { mHistoryLength = length; }
    
    T update( T newValue ) {
        mValues.emplace_back( newValue );
        while ( mValues.size() > mHistoryLength ) {
            mValues.pop_front();
        }
        T sum = std::accumulate( mValues.begin(), mValues.end(), static_cast<T>( 0 ) );
        mCurrentAverage = sum / std::min( mHistoryLength, mValues.size() );
        
        return mCurrentAverage;
    }
    
    T getCurrent() { return mCurrentAverage; }
    
    std::deque<T> mValues;
    size_t mHistoryLength;
    T mCurrentAverage;
};

} // namespace whg
