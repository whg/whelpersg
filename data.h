#pragma once

#include <vector>
#include <deque>
#include <unordered_map>

template<typename T>
inline std::ostream& operator<<(std::ostream &os, const std::vector<T> &vec) {
	os << "[";
	for (int i = 0; i < vec.size(); i++) {
		os << vec[i] << (i == (vec.size()-1) ? "" : ", ");
	}
	return os << "]";
}

template<typename K, typename V>
inline std::ostream& operator<<(std::ostream &os, const std::unordered_map<K, V> &map) {
	os << "{";
	uint i = 0;
	for (auto it = map.begin(); it != map.end(); ++it) {
		auto pair = *it;
		
		os << pair.first << ": " << pair.second << (++i == map.size() ? "" : ", ");
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
		T output;
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


} // namespace whg