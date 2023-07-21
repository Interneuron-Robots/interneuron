/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-18 14:58:53
 * @LastEditTime: 2023-07-20 16:56:25
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
#include <cstdint>
#include <cassert>
#include <tuple>
#include "tools.hpp"

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
	enum class FusionPolicy
	{
 		EXACT_TIME,
  		ALL_AVAILABLE
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
		void add_log(std::string timepoint, uint64_t t0, uint64_t t1, uint64_t t2=0){
			logs[timepoint] = std::make_tuple(t0, t1, t2);// usually, t0 si the current time ,t1 is the reference time
		}
		std::string output_log(){
			std::string res = "\"last_sample\":"+std::to_string(last_sample_time_)+",\"this_sample\":"+std::to_string(this_sample_time_)+",\"remain\":"+std::to_string(remain_time_)+",\"records\":[";
			for(auto it = logs.begin(); it != logs.end(); it++){
				res += "\"timepoint_id\":\""+it->first + "\",\"record\":[" + std::to_string(std::get<0>(it->second)) + "," + std::to_string(std::get<1>(it->second)) + "," + std::to_string(std::get<2>(it->second)) + "]},";
			}
			res[res.size()-1] = ']';//replace the last ','
			
			return res;
		}
		std::map<std::string,std::tuple<uint64_t,uint64_t,uint64_t>> logs;//key is the id of TimePoint, value is the reference time of the point
		#endif
	};

	INTERNEURON_PUBLIC
	class TimePoint
	{
		public:
		TimePoint(){};
		~TimePoint(){};

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