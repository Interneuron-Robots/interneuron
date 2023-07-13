/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-09 21:42:08
 * @LastEditTime: 2023-07-12 17:00:07
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON_LIB__MIDDLE_TIME_POINT_HPP_
#define INTERNEURON_LIB__MIDDLE_TIME_POINT_HPP_
#include "time_point.hpp"
namespace interneuron
{
	class MiddleTimePoint : public TimePoint{
		public:
		MiddleTimePoint(const std::vector<std::string>&sensor_names){
			for(auto it = sensor_names.begin(); it != sensor_names.end(); it++){
				reference_times_.insert(std::pair<std::string, uint64_t>(*it, 0));
			}
		}

		//only this is thread safe
		Policy update_reference_times(std::map<std::string, TP_Info>&tp_infos, uint64_t update_ratio=30, uint64_t qos_ratio=30){
			auto now_time = get_timestamp_in_ns();
			Policy policy = Policy::QualityFirst;
			lock();
			for(auto it = tp_infos.begin(); it != tp_infos.end(); it++){
				auto new_policy = update_reference_time(it->first, now_time, it->second, update_ratio, qos_ratio);
				if(new_policy == Policy::Error){
					std::cout<<"Error in update_reference_times"<<std::endl;
					assert(false);
				}
				#ifdef PRINT_DEBUG
				std::cout<<"[Middle]sensor:"<<it->first<<" policy:";
				switch(new_policy){
					case Policy::QualityFirst:
						std::cout<<"QualityFirst"<<std::endl;
						break;
					case Policy::SpeedFirst:
						std::cout<<"SpeedFirst"<<std::endl;
						break;
					case Policy::Emergency:
						std::cout<<"Emergency"<<std::endl;
						break;
					default:
						std::cout<<"Error"<<std::endl;
						break;
				}
				#endif
				if(new_policy > policy){
					policy = new_policy;
				}
			}
			unlock();
			return policy;
		}

		private:
		//may need to change tp_info's value
		Policy update_reference_time(const std::string& sensor_name, const uint64_t now_time, TP_Info& tp_info, uint64_t update_ratio, uint64_t qos_ratio){
			auto it = reference_times_.find(sensor_name);
			if (it == reference_times_.end()){
				assert(false);
				return Policy::Error;
			}else{
				auto new_time = now_time - tp_info.last_sample_time_;
				//first time
				if(it->second == 0){
					it->second = new_time;
					return Policy::QualityFirst;
				}

				auto old_time = it->second;
				
				it->second = (old_time * (100 - update_ratio) + new_time * update_ratio)/100;
				#ifdef PRINT_DEBUG
				std::cout<<"[Middle]old reference time:"<<old_time<<", new reference time:"<<new_time<<" updated:"<<it->second<<std::endl;
				#endif
				if (old_time >= new_time){
					return Policy::QualityFirst;
				}
				auto gap = new_time - old_time;
				if(gap > tp_info.remain_time_){// has used all the remain_time
					return Policy::Emergency;
				}else if(gap > tp_info.remain_time_*qos_ratio/100){
					return Policy::SpeedFirst;
				}
				return Policy::QualityFirst;
			}
		}
		// the key for these maps is the sensor's name
		std::map<std::string, uint64_t> reference_times_;
	};
}
#endif  // INTERNEURON_LIB__MIDDLE_TIME_POINT_HPP_