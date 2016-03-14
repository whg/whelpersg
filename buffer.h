#pragma once

#include <vector>
#include <algorithm>

template <typename T>
class RingBuffer {
public:
	RingBuffer(): mHead(0), mTail(0) {}
	RingBuffer(size_t size): RingBuffer() {
		resize(size);
	}
	
	void resize(size_t n) {
		mSize = n;
		data.resize(mSize);
	}
	
	size_t size() {
		auto tail = mTail;
		if (tail < mHead) {
			tail+= mSize;
		}
		return tail - mHead;
	}
	
	size_t capacity() {
		return mSize;
	}
	
	size_t available() {
		return capacity() - size();
	}
	
	bool empty() {
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
	
	T peek() {
		return data[mHead];
	}
	
	T pop() {
		T temp = data[mHead];
		mHead = (mHead + 1) % mSize;
		return temp;
	}
	
	bool push(const std::vector<T> &vs) {
		if (available() >= vs.size() + 1) {
			auto firstChunk = std::min(vs.size(), mSize - mTail);
			
			std::copy(vs.begin(), vs.begin() + firstChunk, &data[mTail]);
			mTail = (mTail + firstChunk) % mSize;
			
			if (firstChunk != vs.size()) {
				std::copy(vs.begin() + firstChunk, vs.end(), &data[mTail]);
			}
			return true;
		}
		return false;
	}
	
	void print() {
#include <iostream>
		std::cout << "[";
		for (int i =0 ; i < data.size(); i++) {
			std::cout << data[i] << (i == (data.size()-1) ? "" : ", ");
		}
		
		std::cout << "]" << std::endl;
	}
	
protected:
	std::vector<T> data;
	size_t mHead, mTail, mSize;
	
	friend std::ostream& operator<<(std::ostream &os, RingBuffer<T> const &rb);
};

template<typename T>
std::ostream& operator<<(std::ostream &os, RingBuffer<T> const &rb) {
	os << "[";
	for (int i =0 ; i < rb.data.size(); i++) {
		os << rb.data[i] << (i == (rb.data.size()-1) ? "" : ", ");
	}
	return os << "]";
}