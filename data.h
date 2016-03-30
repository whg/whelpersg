#pragma once

#include <vector>

template<typename T>
inline std::ostream& operator<<(std::ostream &os, std::vector<T> const &vec) {
	os << "[";
	for (int i =0 ; i < vec.size(); i++) {
		os << vec[i] << (i == (vec.size()-1) ? "" : ", ");
	}
	return os << "]";
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

} // namespace whg