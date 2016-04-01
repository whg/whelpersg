#pragma once

#include <vector>
#include <algorithm>
#include <atomic>

template <typename T>
struct BaseData {
	std::vector<T> data;
	size_t mSize;
};

template <typename T>
class RingBuffer : public BaseData<T> {
public:
	RingBuffer(size_t size=1024): mHead(0), mTail(0) {
		resize(size);
	}
	
	void resize(size_t n) {
		this->mSize = n;
		this->data.resize(this->mSize);
	}
	
	size_t size() const {
		auto tail = mTail;
		if (tail < mHead) {
			tail+= this->mSize;
		}
		return tail - mHead;
	}
	
	size_t capacity() const {
		return this->mSize;
	}
	
	size_t available() const {
		return capacity() - size();
	}
	
	bool empty() const {
		return size() == 0;
	}
	
	bool push(T v) {
		if (available() >= 2) {
			this->data[mTail] = v;
			mTail = (mTail + 1) % this->mSize;
			return true;
		}
		return false;
	}
	
	T peek(size_t offset=0) const {
		return this->data[(mHead + offset) % this->mSize];
	}
	
	T pop() {
		T temp = this->data[mHead];
		mHead = (mHead + 1) % this->mSize;
		return temp;
	}
	
	void discard(size_t n=1) {
		// TODO: raise an exception if we try to discard too much?
		auto nextHead = (mHead + n) % this->mSize;
		mHead = nextHead;
	}
	
	bool push(const T* const vs, size_t N) {
		if (available() >= N + 1) {
			auto firstChunk = std::min(N, this->mSize - mTail);
			
			std::copy(vs, vs + firstChunk, &this->data[mTail]);
			mTail = (mTail + firstChunk) % this->mSize;
			
			if (firstChunk != N) {
				std::copy(vs + firstChunk, vs + N, &this->data[mTail]);
				mTail = (mTail + (N - firstChunk)) % this->mSize;
			}
			return true;
		}
		return false;
	}
	
	bool push(const std::vector<T> &vs) {
		return push(&vs[0], vs.size());
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
	size_t mHead, mTail;
	
	friend std::ostream& operator<<(std::ostream &os, RingBuffer<T> const &rb);
};

template<typename T>
inline std::ostream& operator<<(std::ostream &os, BaseData<T> const &bd) {
	os << "[";
	for (int i =0 ; i < bd.data.size(); i++) {
		os << bd.data[i] << (i == (bd.data.size()-1) ? "" : ", ");
	}
	return os << "]";
}

template<typename T>
class AtomicSingleQueue : protected BaseData<T> {
public:
	
	AtomicSingleQueue(size_t size): mHead(0), mTail(0) {
		this->mSize = size;
		this->data.resize(size);
	}
	
	bool push(T &&value) {
		auto writePos = mTail.load();
		auto nextWritePos = (writePos + 1) % this->mSize;
		
		if (nextWritePos != mHead.load()) {
			this->data[writePos] = value;
			mTail.store(nextWritePos);
			return true;
		}
		return false;
	}
	
	bool pop(T &output) {
		auto readPos = mHead.load();
		if (readPos == mTail.load()) {
			return false;
		}
		
		auto nextReadPos = (readPos + 1) % this->mSize;
		
		output = std::move(this->data[readPos]);
		mHead.store(nextReadPos);
		return true;
	}
	
	bool empty() {
		return mHead.load() == mTail.load();
	}
	
	size_t size() const {
		auto tail = mTail.load();
		auto head = mHead.load();
		if (tail < head) {
			tail+= this->mSize;
		}
		return tail - head;
	}
	
protected:

	std::atomic<size_t> mHead, mTail;

};
