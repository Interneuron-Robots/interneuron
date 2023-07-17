/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-18 14:58:53
 * @LastEditTime: 2023-07-16 19:58:09
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON_LIB__TIME_POINT_HPP_
#define INTERNEURON_LIB__TIME_POINT_HPP_
#include <map>
#include <string>
#include <mutex>
#include "visibility_control.hpp"
#include <memory>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <cassert>
#include <tuple>
namespace interneuron
{
	//the order shouldn't be changed
	INTERNEURON_PUBLIC
	enum class Policy
	{
		QualityFirst,
		SpeedFirst,
		Emergency,
		Error,//usually assert
	};
	INTERNEURON_PUBLIC
	enum class TimePointType
	{
		Source,
		Middle,
		Sink,
	};

	INTERNEURON_PUBLIC
	struct TP_Info
	{
		uint64_t this_sample_time_;
		uint64_t last_sample_time_;
		uint64_t remain_time_; // with the reference time in timepoint and remain_time in message_info, you can know the deadline
		TP_Info() : this_sample_time_(0), last_sample_time_(0), remain_time_(0) {}
		TP_Info(uint64_t this_sample_time, uint64_t last_sample_time, uint64_t remain_time) : this_sample_time_(this_sample_time), last_sample_time_(last_sample_time), remain_time_(remain_time) {}
		std::string print(){
			return "this_sample_time:"+std::to_string(this_sample_time_)+", last_sample_time:"+std::to_string(last_sample_time_)+", remain_time:"+std::to_string(remain_time_);
		}
		#ifdef RECORD_LOG
		void add_log(std::string timepoint, uint64_t current_time, uint64_t reference_time, uint64_t for_late_use=0){
			logs[timepoint] = std::make_tuple(current_time, reference_time, for_late_use);
		}
		std::map<std::string,std::tuple> logs;//key is the id of TimePoint, value is the reference time of the point
		#endif
	};

	INTERNEURON_PUBLIC
	class TimePoint
	{
		public:
		TimePoint(){};
		~TimePoint(){};

		uint64_t get_timestamp_in_ns()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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

	};
}
#endif