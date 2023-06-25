/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-10 17:26:18
 * @LastEditTime: 2023-06-20 23:21:00
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
        std::cout << "key already exists" << std::endl;
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

void set_deadline(uint64_t deadline){
    std::lock_guard<std::mutex> lock(this->mtx_);
    this->deadline_ = deadline;
}
todo: use deadline instead of remain_time, remain_time seems useless
// return true if deadline miss happens
bool TimePointManager::update_remain_time(uint64_t new_time, uint8_t x){
    std::lock_guard<std::mutex> lock(this->mtx_);
    if (this->remain_time_reference_ == 0){
        this->remain_time_reference_ = new_time;
        return true;
    }
    this->remain_time_reference_ = (this->remain_time_reference_ * (100 - x) + new_time * x) / 100;
    return true;
}

uint64_t TimePointManager::get_remain_time(){
    std::lock_guard<std::mutex> lock(this->mtx_);
    return this->remain_time_reference_;
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