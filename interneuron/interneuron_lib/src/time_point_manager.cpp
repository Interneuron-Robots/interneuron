/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-10 17:26:18
 * @LastEditTime: 2023-05-23 10:37:37
 * @LastEditors: Sauron
 */
#include "interneuron_lib/time_point_manager.hpp"
#include <iostream>
using namespace interneuron;

bool TimePointManager::add_timepoint(const std::string &topic_name, const std::string &node_name, const std::vector<std::string> &sensor_names){
	    std::lock_guard<std::mutex> lock(this->mtx_);
    if (this->time_points_.find(topic_name) == this->time_points_.end()){
	this->time_points_.insert(std::pair<std::string, std::map<std::string, std::shared_ptr<TimePoint>>>(topic_name, std::map<std::string, std::shared_ptr<TimePoint>>()));
    } 
    if (this->time_points_[topic_name].find(node_name) == this->time_points_[topic_name].end()){
	this->time_points_[topic_name][node_name]= std::make_shared<TimePoint>();
    }else{
	std::cout << "node_name already exists" << std::endl;
	return false;
    }
    auto tp = this->time_points_[topic_name][node_name];
    tp->lock();
    for (auto &sensor_name : sensor_names){
	tp->reference_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	tp->remain_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	tp->wait_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	tp->last_sample_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
    }
    tp->unlock();
    //initialize the value using update_time() manually if needed. In most cases, the value will be updated automatically.
    return true;
}

std::shared_ptr<TimePoint> TimePointManager::get_timepoint(const std::string &topic_name, const std::string &node_name){
    std::lock_guard<std::mutex> lock(this->mtx_);
    auto it = this->time_points_.find(topic_name);
    if (it == this->time_points_.end()){
    std::cout << "topic_name not found" << std::endl;
    return nullptr;
    }
    auto tp = it->second.find(node_name);
    if (tp == it->second.end()){
    std::cout << "node_name not found" << std::endl;
    return nullptr;
    }

    return tp->second;
}

bool TimePointManager::default_update(const std::string &topic_name, const std::string& node_name){
    auto tp = this->get_timepoint(topic_name, node_name);
    if (tp == nullptr){
        std::cout << "the timepoint doesnt exist" << std::endl;
        return false;
    }
    tp->lock();
    //todo, should use the message_info to update the timepoint
    //for each sensor, update related timestamp
    //tp->update_time();
    tp->unlock();
    return true;
}