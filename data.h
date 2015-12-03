#pragma once

template<typename T>
class Smoother {
    int reach;
    std::deque<T> data;
    T current;
public:
    Smoother() : reach(3) {

    }

    void setReach(int n) { reach = n; }
    T& getCurrent() { return current; }

    void add(const T const &point) {
        data.push_back(point);

        while (data.size() > reach) {
            data.pop_front();
        }

        calcCurrent();
    }

    void calcCurrent() {
        T sum = 0;
        for (auto &p : data) {
            sum += p;
        }
        current = sum / float(reach);
    }

};