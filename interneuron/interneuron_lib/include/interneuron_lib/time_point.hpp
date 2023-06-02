/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-18 14:58:53
 * @LastEditTime: 2023-06-01 11:07:56
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON_LIB__TIME_POINT_HPP_
#define INTERNEURON_LIB__TIME_POINT_HPP_
#include <map>
#include <string>
#include <mutex>
#include "visibility_control.hpp"
#include <memory>
#define INITIAL_COUNT_THRESHOLD 10
namespace interneuron
{
	/*
	enum class MonitorTime
	{
		ReferenceTime,
		RemainTime,
		WaitTime,
		LastSample,
	};*/
	enum class Policy
	{
		QualityFirst,
		SpeedFirst,
		Emergency,
		Error,
	};
	INTERNEURON_PUBLIC
	struct TimePoint
	{
		TimePoint(){};
		~TimePoint(){};

//todo: judge whether the current time miss the deadline, if so, you should not update the time maybe
		// this function is not thread-safe, you should lock the mtx_ manually 
		/*bool update_time(const std::string& sensor_name, uint64_t new_time, uint8_t x, MonitorTime target)
		{
			switch (target)
			{
			case MonitorTime::ReferenceTime:
			{
				auto it = this->reference_times_.find(sensor_name);
				if (it == this->reference_times_.end()){
					return false;
				}else{
					it->second = (it->second * (100 - x) + new_time * x) / 100;
				}
				return true;
			}
			case MonitorTime::RemainTime:
			{
				auto it = this->remain_times_.find(sensor_name);
				if (it == this->remain_times_.end()){
					return false;
				}else{
					it->second = (it->second * (100 - x) + new_time * x) / 100;
				}
				return true;
			}
			case MonitorTime::WaitTime:
			{
				auto it = this->wait_times_.find(sensor_name);
				if (it == this->wait_times_.end()){
					return false;
				}else{
					it->second = (it->second * (100 - x) + new_time * x) / 100;
				}
				return true;
			}
			case MonitorTime::LastSample:
			{
				auto it = this->last_sample_times_.find(sensor_name);
				if (it == this->last_sample_times_.end()){
					return false;
				}else{
					it->second = (it->second * (100 - x) + new_time * x) / 100;
				}
				return true;
			}
			}
		};*/
		Policy update_reference_time(const std::string& sensor_name, uint64_t new_time, uint8_t x, uint64_t remain_time){
			if(this->initial_count_<INITIAL_COUNT_THRESHOLD){
				this->initial_count_++;
				x = 50;//update quickly at first, maybe no need to do this
			}
			auto it = this->reference_times_.find(sensor_name);
			if (it == this->reference_times_.end()){
				return Policy::Error;
			}else{
				auto old_time = it->second;
				it->second = (old_time * (100 - x) + new_time * x) / 100;
				if (new_time - old_time > remain_time){
					return Policy::SpeedFirst;
				}else{
					return Policy::QualityFirst;
				}
			}
		}
		uint64_t update_last_sample_time(uint64_t new_time){
			auto old_time = this->last_sample_time_;
			this->last_sample_time_ = new_time;
			return old_time;
		}
		void lock()
		{
			this->mtx_.lock();
		}
		void unlock()
		{
			this->mtx_.unlock();
		}
		// we dont set lock for each mutex because we think a TimePoint is seldom accessed concurrently and the update for these values usually comes from the same try
		std::mutex mtx_;
		// the key for these maps is the sensor's name
		std::map<std::string, uint64_t> reference_times_;

		//std::map<std::string, uint64_t> remain_times_;	    // key is sensor name
		//std::map<std::string, uint64_t> wait_times_;	    // key is sensor name
		//for sensor start point
		//uint64_t last_sample_time_;

		int initial_count_ = 0;
		//std::map<std::string, uint64_t> last_sample_times_; // for sensor timepoint
	};
}
#endif