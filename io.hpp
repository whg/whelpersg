#pragma once

#include "whelpersg/json/json.hpp"

#include <memory>
#include <fstream>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

namespace whg {
    
    class DataStore {
    public:
        DataStore(std::string jsonpath, int updatePeriod=1000)
        : mJSONPath(jsonpath), mUpdatePeriod(updatePeriod), mIsAlive(true) {
            
            load();
            mUpdateThread = std::thread(&DataStore::update, this);
            mUpdateThread.detach();
        }
        
        ~DataStore() {
            mIsAlive.store(false);
        }
        
        void load() {
            std::ifstream f(mJSONPath);
            
            std::unique_lock<std::mutex> lock(mMutex);
            mData = nlohmann::json::parse(f);
        }
        
        void update() {
            while (mIsAlive.load()) {
                load();
                std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(mUpdatePeriod));
            }
        }
        
        nlohmann::json::basic_json::const_reference operator[](std::string key) const {
            mMutex.lock();
            nlohmann::json::basic_json::const_reference output = mData[key];
            mMutex.unlock();
            return output;
        }
        
        nlohmann::json data() const {
            return mData;
        }
        
        
    protected:
        std::string mJSONPath;
        nlohmann::json mData;
        mutable std::mutex mMutex;
        std::thread mUpdateThread;
        std::atomic<bool> mIsAlive;
        int mUpdatePeriod; // in millis
    };
}