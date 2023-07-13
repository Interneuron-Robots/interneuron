/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-10 11:14:13
 * @LastEditTime: 2023-07-12 16:50:59
 * @LastEditors: Sauron
 */
#ifndef INTERNEURON_LIB__SINK_TIME_POINT_HPP_
#define INTERNEURON_LIB__SINK_TIME_POINT_HPP_
#include "time_point.hpp"
#include "source_time_point.hpp"
namespace interneuron{

class SinkTimePoint : public TimePoint
{
	public:
	SinkTimePoint(const std::vector<std::string>&sensor_names, const std::vector<std::shared_ptr<SourceTimePoint>>&source_time_points){
		for(long unsigned int i = 0; i < sensor_names.size(); i++){
			finish_times_.insert(std::pair<std::string, uint64_t>(sensor_names[i], 0));
			source_time_points_.insert(std::pair<std::string, std::shared_ptr<SourceTimePoint>>(sensor_names[i], source_time_points[i]));
		}
	};
	~SinkTimePoint(){};
	void update_finish_times(std::map<std::string, TP_Info>&tp_infos){
		auto new_time = get_timestamp_in_ns();
		for(auto it = tp_infos.begin(); it != tp_infos.end(); it++){
			update_finish_time(it->first, new_time-it->second.last_sample_time_, it->second);
		}
	}
	
	private:
	void update_finish_time(const std::string& sensor_name, uint64_t new_time, TP_Info&tp_info, uint64_t update_ratio=30){
		auto it = finish_times_.find(sensor_name);
		if (it == finish_times_.end()){
			assert(false);
		}else{
			auto source_tp = source_time_points_[sensor_name];
			//check whether a deadline miss happens
			auto deadline = source_tp->get_deadline();
			
			if(new_time > deadline){
				#ifdef PRINT_DEBUG
				std::cout<<"[Sink]sensor:"<<sensor_name<<"'s pipeline costs:"<<new_time<<" reference time is:"<<it->second<<std::endl;
				std::cout<<"deadline miss happens"<<std::endl;
				#endif
				//do something
				source_tp->set_remain_time(source_tp->get_remain_time()*0.8);//make the pipeline tend to be speed first
				return;
			}
			//no deadline miss happens
			
			//first time
			if(it->second == 0){
				it->second = new_time;
				source_tp->set_remain_time(deadline - new_time);
				return;
			}
			auto old_time = it->second;
			it->second = (old_time * (100 - update_ratio) + new_time * update_ratio)/100;
			//may need to change the remain time of source time point
			#ifdef PRINT_DEBUG
			std::cout<<"[Sink]sensor:"<<sensor_name<<"'s pipeline costs:"<<new_time<<" reference time is:"<<old_time<<" updated:"<<it->second<<std::endl;
			#endif
			}
	}
	std::map<std::string, uint64_t>finish_times_;
	std::map<std::string, std::shared_ptr<SourceTimePoint>>source_time_points_;
	};
}
#endif  // INTERNEURON_LIB__SINK_TIME_POINT_HPP_