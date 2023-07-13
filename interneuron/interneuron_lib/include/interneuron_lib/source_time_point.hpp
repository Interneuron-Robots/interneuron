/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-09 18:44:57
 * @LastEditTime: 2023-07-12 17:15:00
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
	//pay attention, here the deadline means the deadline since the last_sample_time
	SourceTimePoint(uint64_t deadline, uint64_t period){
		deadline_ = deadline;
		period_ = period;
		last_sample_time_ = 0;
		remain_time_ = 0;//the initialisation of remain_time_ is done by the sink
		#ifdef PRINT_DEBUG
		std::cout<<"[Source][init]deadline:"<<deadline_<<", period:"<<period_<<", remain_time:"<<remain_time_<<std::endl;
		#endif
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
		#ifdef PRINT_DEBUG
		std::cout<<"[Source][set_remain_time]new remain_time:"<<remain_time<<" old remain_time:"<<remain_time_<<std::endl;
		#endif
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
			#ifdef PRINT_DEBUG
			std::cout<<"[Source][first]"<<tp_info.print()<<std::endl;
			#endif
			return Policy::QualityFirst;
		}
		auto time_diff = now_time - last_sample_time_;
		tp_info.last_sample_time_ = last_sample_time_;
		tp_info.this_sample_time_ = now_time;
		tp_info.remain_time_ = remain_time_;//remain_time_ is updated by SinkTimePoint
		last_sample_time_ = now_time;
		if(period_>time_diff){
			#ifdef PRINT_DEBUG
			std::cout<<"[Source][normal]sample time:"<<time_diff<<", "<<tp_info.print()<<", policy:QualityFirst"<<std::endl;
			#endif
			return Policy::QualityFirst;
		}else if(time_diff-period_>remain_time_){//must check remain_time_ first
			#ifdef PRINT_DEBUG
			std::cout<<"[Source][normal]sample time:"<<time_diff<<", "<<tp_info.print()<<", policy:Emergency"<<std::endl;
			#endif
			return Policy::Emergency;
		}else if(time_diff-period_>period_*qos_ratio/100){
			#ifdef PRINT_DEBUG
			std::cout<<"[Source][normal]sample time:"<<time_diff<<", "<<tp_info.print()<<", policy:SpeedFirst"<<std::endl;
			#endif
			return Policy::SpeedFirst;
		}else{
			#ifdef PRINT_DEBUG
			std::cout<<"[Source][normal]sample time:"<<time_diff<<", "<<tp_info.print()<<", policy:QualityFirst"<<std::endl;
			#endif
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