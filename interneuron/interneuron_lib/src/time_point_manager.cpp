/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-10 17:26:18
 * @LastEditTime: 2023-07-11 23:29:36
 * @LastEditors: Sauron
 */
#include "interneuron_lib/time_point_manager.hpp"
#include <iostream>
#include <cassert>
using namespace interneuron;

std::shared_ptr<SourceTimePoint> TimePointManager::add_source_timepoint(const std::string &sensor_name, uint64_t deadline, uint64_t period){
	std::lock_guard<std::mutex> lock(this->source_mtx_);
    if (this->source_time_points_.find(sensor_name) == this->source_time_points_.end()){
        this->source_time_points_[sensor_name]= std::make_shared<SourceTimePoint>(deadline, period);
    }else{
        assert(false);
        return nullptr;
    } 
    return this->source_time_points_[sensor_name];
}

std::shared_ptr<MiddleTimePoint> TimePointManager::add_middle_timepoint(const std::string &key, const std::vector<std::string> &sensor_names){
    std::lock_guard<std::mutex> lock(this->middle_mtx_);
    if (this->middle_time_points_.find(key) == this->middle_time_points_.end()){
        this->middle_time_points_[key]= std::make_shared<MiddleTimePoint>(sensor_names);
    }else{
        assert(false);
        return nullptr;
    } 
    return this->middle_time_points_[key];
}

std::shared_ptr<SinkTimePoint> TimePointManager::add_sink_timepoint(const std::string &key, const std::vector<std::string> &sensor_names){
    std::lock_guard<std::mutex> lock(this->sink_mtx_);
    if (this->sink_time_points_.find(key) == this->sink_time_points_.end()){
        std::vector<std::shared_ptr<SourceTimePoint>> source_time_points;
        for(auto&sensor_name:sensor_names){
            auto source_time_point = std::static_pointer_cast<SourceTimePoint>(this->get_timepoint(sensor_name, TimePointType::Source));
            if(source_time_point == nullptr){
                assert(false);
                return nullptr;
            }
            source_time_points.push_back(source_time_point);
        }
        this->sink_time_points_[key]= std::make_shared<SinkTimePoint>(sensor_names, source_time_points);
    }else{
        assert(false);
        return nullptr;
    } 
    return this->sink_time_points_[key];
}

std::shared_ptr<TimePoint> TimePointManager::get_timepoint(const std::string &key, TimePointType type){
    switch(type){
        case TimePointType::Source:{
            std::lock_guard<std::mutex> lock(this->source_mtx_);
            if (this->source_time_points_.find(key) == this->source_time_points_.end()){
                assert(false);
                return nullptr;
            }else{
                return this->source_time_points_[key];
            }
            break;
            }
        case TimePointType::Middle:{
            std::lock_guard<std::mutex> lock(this->middle_mtx_);
            if (this->middle_time_points_.find(key) == this->middle_time_points_.end()){
                assert(false);
                return nullptr;
            }else{
                return this->middle_time_points_[key];
            }
            break;
        }
        case TimePointType::Sink:{
            std::lock_guard<std::mutex> lock(this->sink_mtx_);
            if (this->sink_time_points_.find(key) == this->sink_time_points_.end()){
                assert(false);
                return nullptr;
            }else{
                return this->sink_time_points_[key];
            }
            break;
        }
        default:
            assert(false);
            return nullptr;
    }

}