#pragma once

#include <algorithm>

namespace whg {

template<class T>
size_t argmax(const T &iterable) {
	size_t output;
	size_t i = 0;
	typename T::value_type max = 0;
	for (const auto &v : iterable) {
		if (v > max) {
			max = v;
			output = i;
		}
		i++;
	}
	return output;
}

template<class InputIterator>
size_t argmax(InputIterator begin, InputIterator end) {
	size_t output;
	size_t i = 0;
	typename InputIterator::value_type max = 0;
	for (; begin != end; ++begin) {
		if (*begin > max) {
			max = *begin;
			output = i;
		}
		i++;
	}
	return output;
}

template<class InputIterator>
std::vector<size_t> argsort(InputIterator begin, InputIterator end) {
	
	size_t N = static_cast<size_t>(end - begin);
	std::vector<size_t> output(N);
	for (size_t i = 0; i < N; i++) output[i] = i;
	
	std::sort(output.begin(), output.end(), [&begin](size_t a, size_t b) {
		return *(begin+a) < *(begin+b);
	});
	
	return output;
}


}
