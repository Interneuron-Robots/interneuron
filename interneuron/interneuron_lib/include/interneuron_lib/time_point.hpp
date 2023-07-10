/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-05-18 14:58:53
 * @LastEditTime: 2023-07-09 22:22:29
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
namespace interneuron
{
	//the order shouldn't be changed
	enum class Policy
	{
		QualityFirst,
		SpeedFirst,
		Emergency,
		Error,//usually assert
	};

	INTERNEURON_PUBLIC
	struct TP_Info
	{
		uint64_t this_sample_time_;
		uint64_t last_sample_time_;
		uint64_t remain_time_; // with the reference time in timepoint and remain_time in message_info, you can know the deadline
		TP_Info(uint64_t this_sample_time, uint64_t last_sample_time, uint64_t remain_time) : this_sample_time_(this_sample_time), last_sample_time_(last_sample_time), remain_time_(remain_time) {}
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