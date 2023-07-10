/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-09 18:44:57
 * @LastEditTime: 2023-07-09 22:19:03
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
		last_sample_time = 0;
	}
	Policy update_sample_time(float qos_ratio=0.3){
		auto now_time = get_timestamp_in_ns();
		//first time
		if(last_sample_time == 0){
			last_sample_time = now_time;
			return Policy::QualityFirst;
		}
		auto time_diff = now_time - last_sample_time;
		last_sample_time = now_time;
		if(period_>time_diff){
			return Policy::QualityFirst;
		}else if(time_diff-period_>remain_time_){//must check remain_time_ first
			return Policy::Emergency;
		}else if(time_diff-period_>static_cast<float>(period_)*qos_ratio){
			return Policy::SpeedFirst;
		}else{
			return Policy::QualityFirst;
		}
	}
	private:
	uint64_t deadline_;
	uint64_t period_;
	uint64_t remain_time_;// the strictest remain_time if multiple sinks exist
	uint64_t last_sample_time;
}
}
#endif  // INTERNEURON_LIB__SOURCE_TIME_POINT_HPP_