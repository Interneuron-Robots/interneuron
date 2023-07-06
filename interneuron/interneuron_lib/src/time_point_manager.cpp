/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-10 17:26:18
 * @LastEditTime: 2023-07-05 21:47:42
 * @LastEditors: Sauron
 */
#include "interneuron_lib/time_point_manager.hpp"
#include <iostream>
using namespace interneuron;

bool TimePointManager::add_timepoint(const std::string &key, const std::vector<std::string> &sensor_names){
	    std::lock_guard<std::mutex> lock(this->mtx_);
    if (this->time_points_.find(key) == this->time_points_.end()){
        this->time_points_[key]= std::make_shared<TimePoint>();
    }else{
        std::cout << "[Error][TimePointManager::add_timepoint] key:"<<key<<" already exists" << std::endl;
        return false;
    } 
    auto tp = this->time_points_[key];
    tp->lock();
    for (auto &sensor_name : sensor_names){
	tp->reference_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	//tp->remain_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	//tp->wait_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	//tp->last_sample_times_ = 0;
    }
    tp->unlock();
    //initialize the value using update_time() manually if needed. In most cases, the value will be updated automatically.
    return true;
}

void TimePointManager::init_set(std::string sensor_name, uint64_t deadline, uint64_t remain_time){
    std::lock_guard<std::mutex> lock(this->mtx_);
    deadlines_[sensor_name] = deadline;
    remain_times_[sensor_name] = remain_time;
}

//todo: use deadline instead of remain_time, remain_time seems useless
// return true if deadline miss happens
bool TimePointManager::update_remain_time(std::string sensor_name, uint64_t finish_time, uint8_t x){
    std::lock_guard<std::mutex> lock(this->mtx_);
    auto it = this->deadlines_.find(sensor_name);
    auto it2 = this->remain_times_.find(sensor_name);
    if (it == this->deadlines_.end() || it2 == this->remain_times_.end()){
        std::cout << "[Error][TimePointManager::update_remain_time] sensor_name:"<<sensor_name<<" not found" << std::endl;
        return false;//no deadline, so no deadline miss
    }
    if(it->second < finish_time){
        #if CMAKE_BUILD_TYPE == DEBUG
        std::cout << "deadline miss" << std::endl;
        #endif
        return true;
    }
    auto new_time = it->second - finish_time;
    if (it2->second == 0){
        it2->second = new_time;
        return false;
    }
    it2->second = (it2->second * (100 - x) + new_time * x) / 100;
    return false;
}

uint64_t TimePointManager::get_remain_time(std::string sensor_name){
    std::lock_guard<std::mutex> lock(this->mtx_);
    auto it = this->remain_times_.find(sensor_name);
    if (it == this->remain_times_.end()){
        std::cout << "[Error][TimePointManager::get_remain_time] sensor_name:"<<sensor_name<<" not found" << std::endl;
        return 0;
    }
    return it->second;
}

std::shared_ptr<TimePoint> TimePointManager::get_timepoint(const std::string &key){
    std::lock_guard<std::mutex> lock(this->mtx_);
    auto tp = this->time_points_.find(key);
    if (tp == this->time_points_.end()){
    std::cout << "timepoint not found" << std::endl;
    return nullptr;
    }
    return tp->second;
}