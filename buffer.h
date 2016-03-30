#pragma once

#include <vector>
#include <algorithm>

template <typename T>
class RingBuffer {
public:
	RingBuffer(size_t size=1024): mHead(0), mTail(0) {
		resize(size);
	}
	
	void resize(size_t n) {
		mSize = n;
		data.resize(mSize);
	}
	
	size_t size() const {
		auto tail = mTail;
		if (tail < mHead) {
			tail+= mSize;
		}
		return tail - mHead;
	}
	
	size_t capacity() const {
		return mSize;
	}
	
	size_t available() const {
		return capacity() - size();
	}
	
	bool empty() const {
		return size() == 0;
	}
	
	bool push(T v) {
		if (available() >= 2) {
			data[mTail] = v;
			mTail = (mTail + 1) % mSize;
			return true;
		}
		return false;
	}
	
	T peek(size_t offset=0) const {
		return data[(mHead + offset) % mSize];
	}
	
	T pop() {
		T temp = data[mHead];
		mHead = (mHead + 1) % mSize;
		return temp;
	}
	
	void discard(size_t n=1) {
		// TODO: raise an exception if we try to discard too much?
		auto nextHead = (mHead + n) % mSize;
//		if (nextHead <= mTail) {
			mHead = nextHead;
//		}
//		else {
//			std::cout << "asdfasd" << std::endl;
//		}
	}
	
	bool push(const T* const vs, size_t N) {
		if (available() >= N + 1) {
			auto firstChunk = std::min(N, mSize - mTail);
			
			std::copy(vs, vs + firstChunk, &data[mTail]);
			mTail = (mTail + firstChunk) % mSize;
			
			if (firstChunk != N) {
				std::copy(vs + firstChunk, vs + N, &data[mTail]);
				mTail = (mTail + (N - firstChunk)) % mSize;
			}
			return true;
		}
		return false;
	}
	
	bool push(const std::vector<T> &vs) {
		return push(&vs[0], vs.size());
//		if (available() >= vs.size() + 1) {
//			auto firstChunk = std::min(vs.size(), mSize - mTail);
//			
//			std::copy(vs.begin(), vs.begin() + firstChunk, &data[mTail]);
//			mTail = (mTail + firstChunk) % mSize;
//			
//			if (firstChunk != vs.size()) {
//				std::copy(vs.begin() + firstChunk, vs.end(), &data[mTail]);
//			}
//			return true;
//		}
//		return false;
	}
	
	
	
	std::vector<T> grab(size_t n) const {
		size_t N = std::min(size(), n);
		std::vector<T> output(N);
		for (size_t i = 0; i < N; i++) {
			output[i] = peek(i);
		}
		return output;
	}
	
protected:
	std::vector<T> data;
	size_t mHead, mTail, mSize;
	
	friend std::ostream& operator<<(std::ostream &os, RingBuffer<T> const &rb);
};

//template<typename T>
inline std::ostream& operator<<(std::ostream &os, RingBuffer<float> const &rb) {
	os << "[";
	for (int i =0 ; i < rb.data.size(); i++) {
		os << rb.data[i] << (i == (rb.data.size()-1) ? "" : ", ");
	}
	return os << "]";
}