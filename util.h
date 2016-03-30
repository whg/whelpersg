#pragma once

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


}
