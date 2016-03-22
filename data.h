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


//#include <valarray>
//
//template<typename T>
//class Smoother {
//    int reach;
//    std::deque<T> data;
//    T current;
//public:
//    Smoother() : reach(3) {
//
//    }
//
//    void setReach(int n) { reach = n; }
//    T& getCurrent() { return current; }
//
//    virtual void add(const T &point) {
//        data.push_back(point);
//
//        while (data.size() > reach) {
//            data.pop_front();
//        }
//
//        calcCurrent();
//    }
//
//    virtual void calcCurrent() {
//        T sum = 0;
//        for (auto &p : data) {
//            sum += p;
//        }
//        current = sum / float(reach);
//    }
//
//};
//
//template<typename T>
//class MultiSmoother : public Smoother {
//protected:
//    std::deque<std::vallarray> data;
//public:
//
//    virtual void add(const T &point) {
//        std::valarray<float> va(&point[0], point.size());
//        Smoother::add(va);
//    }
//
//    virtual void calcCurrent() {
//        
//        T sum = std::vector(data.front().size(), 0);
//        for (auto &iter : data) {
//            for ()
//        }
//    }
//};