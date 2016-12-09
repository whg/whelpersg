#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <regex>
#include <sstream>
#include <functional>

#include "whelpersg/serial.hpp"

struct Reader {
    void add(const std::string &input) {
        mStream << input;
        update();
    }

    void update() {
        constexpr size_t len = 100;
        char line[len];
        size_t numRead = 0;
        while (mStream.getline(line, len).good()) {
            if (line[0] == '[') {
                std::smatch matches;
                std::regex expr("\\[(-?\\d+),(-?\\d+),(-?\\d+)\\]");
                std::regex_match(std::string(line), matches, expr);

                if (mCallback) {
                    try {
                        mCallback(std::stoi(matches[1].str()), std::stoi(matches[2].str()), std::stoi(matches[3].str()));
                    }
                    catch (std::invalid_argument &e) {}
                }
            }
            numRead+= mStream.gcount();
        }

        // don't like this, but stringstream seems a bit funny to work with
        mStream = std::stringstream();
        mStream << line;
    }

    std::stringstream mStream;
    std::function<void(int, int, int)> mCallback;
};


class Something {
public:
    void test(int x, int y, int z) {
        printf("here we are: %d, %d, %d\n", x, y, z);
    }
};


using namespace std;
using namespace std::chrono;

int main(int argc, char *argv[]) {
    Serial serial("tty.usbserial-A102SX3F", 9600);
    Reader reader;
    Something something;

    reader.mCallback = std::bind(&Something::test, &something, placeholders::_1, placeholders::_2, placeholders::_3);

    atomic<bool> working(true);
    auto updateThread = thread([&serial, &working, &reader]() {
        while (working) {
            if (serial.getNumBytesAvailable()) {
                reader.add(serial.readAllCharacters());
            }
        }
    });

    updateThread.detach();

    string input;
    cin >> input;
    working = false;
    this_thread::sleep_for(duration<float, std::milli>(100));

}
