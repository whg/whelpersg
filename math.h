#pragma once

#include <vector>
#include <cassert>

template <typename T>
std::vector<T> dot(const std::vector<T> &a, const std::vector<T> &b) {
	
	auto N = b.size();
	assert(N == a.size());
	
	std::vector<T> output(a.size());
	
	for (size_t i = 0; i < N; i++) {
		output[i] = a[i] * b[i];
	}
	
	return output;
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
			sum = 0;
			auto &row = a[i];
			for (size_t j = 0; j < aCols; j++) {
				sum+= row[j] * b[j];
			}
			output[i] = sum;
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