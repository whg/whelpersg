#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
#include <utility>
#include <limits>

namespace whg {

#define CREATE_VEL_ELEM_OPERATOR(op) \
template<typename T> \
inline std::vector<T> operator op (std::vector<T> const &lhs, std::vector<T> const &rhs) { \
auto N = lhs.size(); \
assert(N == rhs.size()); \
std::vector<T> output(N); \
for (size_t i = 0; i < N; i++) output[i] = lhs[i] op rhs[i]; \
return output; \
}
	
CREATE_VEL_ELEM_OPERATOR(+)
CREATE_VEL_ELEM_OPERATOR(-)
CREATE_VEL_ELEM_OPERATOR(*)
CREATE_VEL_ELEM_OPERATOR(/)

template <typename T>
T dot(const std::vector<T> &a, const std::vector<T> &b) {
	
	auto N = b.size();
	assert(N == a.size());

	T sum = 0;
	
	for (size_t i = 0; i < N; i++) {
		sum+= a[i] * b[i];
	}
	return sum;
}

template <typename T>
std::vector<T> dot(const std::vector<std::vector<T>> &a, const std::vector<T> &b) {
	
	assert(a.size() > 0);
	
	auto N = b.size();
	auto aRows = a.size();
	auto aCols = a[0].size();
	assert(N == aCols || N == aRows);
	
	std::vector<T> output;
	
	if (N == aCols) {
		output.resize(aRows);
		for (size_t i = 0; i < aRows; i++) {
			output[i] = dot(a[i], b);
		}
	}
	else if (N == aRows) {
		output.resize(aCols);
		T sum;
		for (size_t i = 0; i < aCols; i++) {
			sum = 0;
			for (size_t j = 0; j < aRows; j++) {
				sum+= a[j][i] * b[j];
			}
			output[i] = sum;
		}
	}
	
	return output;
}

template <typename T>
std::vector<T> max(const std::vector<T> &input, T maxVal=0) {
	const auto N = input.size();
	std::vector<T> output(N);
	for (size_t i = 0; i < N; i++) {
		output[i] = std::max(input[i], maxVal);
	}
	return output;
}


template <class Iterable>
typename Iterable::value_type sum(const Iterable &input) {
	typename Iterable::value_type output = 0;
	for (const auto &v : input) {
		output+= v;
	}
	return output;
}

template <class Iterable>
typename Iterable::value_type maxValue(const Iterable &input) {
	typename Iterable::value_type output = std::numeric_limits<typename Iterable::value_type>::min();
	for (const auto &v : input) {
		output = std::max(v, output);
	}
	return output;
}

template <class Iterable>
typename Iterable::value_type mean(const Iterable &input) {
	return sum(input) / static_cast<typename Iterable::value_type>(input.size());
}

template <class InputIterator>
typename InputIterator::value_type median(InputIterator begin, InputIterator end) {
	std::vector<typename InputIterator::value_type> copy(begin, end);
	std::sort(copy.begin(), copy.end());
	return copy[copy.size() / 2 + 1];
}

template <typename T>
T variance(const std::vector<T> &input) {
	std::vector<T> squared(input.size());
	std::transform(input.begin(), input.end(), squared.begin(), [](T v) { return v * v; });
	auto m = mean(input);
	return mean(squared) - m * m;
}

template <typename T>
T std(const std::vector<T> &input) {
	return std::sqrt(variance(input));
}

template <typename T>
T rms(const std::vector<T> &input) {
	std::vector<T> squared(input.size());
	std::transform(input.begin(), input.end(), squared.begin(), [](T v) { return v * v; });
	return std::sqrt(mean(squared));
}


/// Holds the start and end (inclusive) of the ranges
struct ConsecutiveMatch {
	std::pair<size_t, size_t> range;
	
	size_t getLength() const { return range.second - range.first + 1; }
	float getCenter() const { return range.first + getLength() * 0.5f; }
};

inline std::ostream& operator<<(std::ostream &os, const ConsecutiveMatch &cm) {
	return os << "(" << cm.range.first << " -> " << cm.range.second << ", " << cm.getLength() << ")";
}

template <typename T>
std::vector<ConsecutiveMatch> consecutives(const std::vector<T> &input, T threshold=0) {
	
	std::vector<ConsecutiveMatch> output;
	ConsecutiveMatch tempMatch;
	bool isOn = false, currentOn = false;
	
	for (size_t i = 0; i < input.size(); i++) {
		currentOn = input[i] > threshold;
		
		if (currentOn && !isOn) {
			tempMatch.range.first = i;
			isOn = true;
		}
		else if(!currentOn && isOn) {
			tempMatch.range.second = i-1;
			output.push_back(tempMatch);
			isOn = false;
		}
	}
	
	if (isOn) {
		tempMatch.range.second = input.size() - 1;
		output.push_back(tempMatch);
	}
	
	return output;

}



} // namespace whg