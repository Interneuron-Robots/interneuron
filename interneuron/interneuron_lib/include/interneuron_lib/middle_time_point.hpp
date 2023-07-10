/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-09 21:42:08
 * @LastEditTime: 2023-07-09 22:22:21
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON_LIB__MIDDLE_TIME_POINT_HPP_
#define INTERNEURON_LIB__MIDDLE_TIME_POINT_HPP_
#include "time_point.hpp"
namespace interneuron
{
	class MiddleTimePoint : public TimePoint{
		public:
		MiddleTimePoint(){}
		// remain time here may need to be replaced by a factor
		Policy update_reference_time(std::string& sensor_name, uint64_t&new_time, uint64_t&remain_time, float update_ratio=0.3, float qos_ratio=0.3){
			auto it = reference_times_.find(sensor_name);
			if (it == reference_times_.end()){
				assert(false);
				return Policy::Error;
			}else{
				//first time
				if(it->second == 0){
					it->second = new_time;
					return Policy::QualityFirst;
				}

				auto old_time = it->second;
				
				it->second = old_time * (1 - update_ratio) + new_time * update_ratio;
				#ifdef PRINT_DEBUG
				std::cout<<"old time:"<<old_time<<", new time:"<<new_time<<" updated time:"<<it->second<<std::endl;
				#endif
				if (old_time >= new_time){
					return Policy::QualityFirst;
				}
				auto gap = new_time - old_time;
				if(gap > remain_time){// has used all the remain_time
					return Policy::Emergency;
				}else if(gap > static_cast<float>(remain_time)*qos_ratio){
					return Policy::SpeedFirst;
				}
				return Policy::QualityFirst;
			}
		}

		Policy update_reference_time(std::string& sensor_name, TP_Info& tp_info, float update_ratio=0.3, float qos_ratio=0.3){
			return update_reference_time(sensor_name, tp_info.this_sample_time_, tp_info.remain_time_, update_ratio, qos_ratio);
		}

		Policy update_reference_times(std::map<std::string, TP_Info>&message_map, float update_ratio=0.3, float qos_ratio=0.3){
			Policy policy = Policy::QualityFirst;
			auto now_time = get_timestamp_in_ns();
			for(auto it = message_map.begin(); it != message_map.end(); it++){
				auto new_policy = update_reference_time(it->first, now_time, update_ratio, qos_ratio);
				if(new_policy == Policy::Error){
					#ifdef PRINT_DEBUG
					std::cout<<"Error in update_reference_times"<<std::endl;
					#endif
					assert(false);
					return Policy::Error;
				}
				#ifdef PRINT_DEBUG
				std::cout<<"sensor:"<<it->first<<" updated policy:"<<new_policy<<std::endl;
				#endif
				if(new_policy > policy){
					policy = new_policy;
				}
			}
			return policy;
		}

		private:
		// the key for these maps is the sensor's name
		std::map<std::string, uint64_t> reference_times_;
	}
}
#endif  // INTERNEURON_LIB__MIDDLE_TIME_POINT_HPP_