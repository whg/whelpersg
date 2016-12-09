#pragma once

#include <algorithm>
#include <string>
#include <sstream>
#include <iterator>

namespace whg {

template<class T>
size_t argmax(const T &iterable) {
	size_t output = 0;
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


/// return a vector of bools where true means that the value at index i is above i+1 and i-1
/// first and last are always false
template<class InputIterator>
std::vector<bool> localmax(InputIterator begin, InputIterator end) {
    
    size_t N = static_cast<size_t>(end - begin);
    std::vector<bool> output(N, false);
    typename InputIterator::value_type lastValue = *begin;

    // TODO: stop at end - 1?
    for (size_t i = 0; begin != end; ++begin, ++i) {
        if (*begin > lastValue && *begin > *std::next(begin)) {
            output[i] = true;
        }
        
        lastValue = *begin;
    }
    
    return output;
}

template <typename T>
T clamp(T value, T min=0, T max=1) {
	return std::min(max, std::max(value, min));
}

template <typename T>
std::string toString(T value) {
	std::stringstream ss;
	ss << value;
	return ss.str();
}


}
