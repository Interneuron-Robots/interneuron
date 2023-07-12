/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-09 18:44:57
 * @LastEditTime: 2023-07-11 22:19:54
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON_LIB__SOURCE_TIME_POINT_HPP_
#define INTERNEURON_LIB__SOURCE_TIME_POINT_HPP_
#include "time_point.hpp"
namespace interneuron
{
	// SourceTimePoint is also responsible for recording metadata for the sensor
class SourceTimePoint : public TimePoint
{
	public:
	SourceTimePoint(uint64_t deadline, uint64_t period){
		deadline_ = deadline;
		period_ = period;
		last_sample_time_ = 0;
		remain_time_ = period_ >= deadline_? period_ - deadline_ : 10000000;//give 10ms space to the first sample
	}
	uint64_t get_deadline(){
		return deadline_;
	}
	uint64_t get_period(){
		return period_;
	}
	uint64_t get_remain_time(){
		return remain_time_;
	}
	void set_remain_time(uint64_t remain_time){
		remain_time_ = remain_time;
	}
	uint64_t get_last_sample_time(){
		return last_sample_time_;
	}
	
	//qos_ratio should be 0~100
	Policy update_sample_time(TP_Info&tp_info, uint64_t qos_ratio=30){
		auto now_time = get_timestamp_in_ns();
		std::lock_guard<std::mutex> lock(mtx_);
		//first time
		if(last_sample_time_ == 0){
			last_sample_time_ = now_time;
			tp_info.this_sample_time_ = now_time;
			tp_info.last_sample_time_ = now_time-period_;
			tp_info.remain_time_ = remain_time_;
			return Policy::QualityFirst;
		}
		auto time_diff = now_time - last_sample_time_;
		tp_info.last_sample_time_ = last_sample_time_;
		tp_info.this_sample_time_ = now_time;
		tp_info.remain_time_ = remain_time_;//remain_time_ is updated by SinkTimePoint
		last_sample_time_ = now_time;
		if(period_>time_diff){
			return Policy::QualityFirst;
		}else if(time_diff-period_>remain_time_){//must check remain_time_ first
			return Policy::Emergency;
		}else if(time_diff-period_>period_*qos_ratio/100){
			return Policy::SpeedFirst;
		}else{
			return Policy::QualityFirst;
		}
	}
	private:
	uint64_t deadline_;
	uint64_t period_;
	uint64_t remain_time_;// the strictest remain_time if multiple sinks exist
	uint64_t last_sample_time_;
};
}
#endif  // INTERNEURON_LIB__SOURCE_TIME_POINT_HPP_