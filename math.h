#pragma once

#include <vector>
#include <cassert>

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
		T sum;
		for (size_t i = 0; i < aRows; i++) {
			output[i] = dot(a[i], b);
//			sum = 0;
//			auto &row = a[i];
//			for (size_t j = 0; j < aCols; j++) {
//				sum+= row[j] * b[j];
//			}
//			output[i] = sum;
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
std::vector<T> max(const std::vector<T> &input, T maxVal) {
	const auto N = input.size();
	std::vector<T> output(N);
	for (size_t i = 0; i < N; i++) {
		output[i] = std::max(input[i], maxVal);
	}
	return output;
}

}