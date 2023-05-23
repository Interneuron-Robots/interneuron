/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-10 17:26:18
 * @LastEditTime: 2023-05-22 23:18:59
 * @LastEditors: Sauron
 */
#include "interneuron_lib/time_point_manager.hpp"
#include <iostream>
using namespace interneuron;

bool TimePointManager::add_timepoint(const std::string &topic_name, const std::string &node_name, std::vector<std::string> &sensor_names){
	    std::lock_guard<std::mutex> lock(this->mtx_);
    auto it = this->time_points_.find(topic_name); 
    if (it == this->time_points_.end()){
	this->time_points_.insert(std::pair<std::string, std::map<std::string, std::shared_ptr<TimePoint>>>(topic_name, std::map<std::string, std::shared_ptr<TimePoint>>()));
    }
    auto tp = it->second.find(node_name);
    if (tp == it->second.end()){
	it->second.insert(std::pair<std::string, std::shared_ptr<TimePoint>>(node_name, std::make_shared<TimePoint>()));
    }else{
	std::cout << "node_name already exists" << std::endl;
	return false;
    }
    auto tpp = it->second.find(node_name)->second;
    tpp->lock();
    for (auto &sensor_name : sensor_names){
	tpp->reference_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	tpp->remain_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	tpp->wait_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
	tpp->last_sample_times_.insert(std::pair<std::string, uint64_t>(sensor_name, 0));
    }
    tpp->unlock();
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
}