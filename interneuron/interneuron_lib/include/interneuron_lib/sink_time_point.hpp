/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-07-10 11:14:13
 * @LastEditTime: 2023-07-18 16:50:10
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
	SinkTimePoint(const std::vector<std::string>&sensor_names, const std::vector<std::shared_ptr<SourceTimePoint>>&source_time_points,const std::string& log_path="~/interneuron_log"){
		#ifdef RECORD_LOG
		log_dir_ = log_path;
		ofs_ = create_file(log_dir_, std::to_string(get_timestamp_in_ns()));
		ofs_<<"{\"meta\":[";
		#endif
		for(long unsigned int i = 0; i < sensor_names.size(); i++){
			finish_times_.insert(std::pair<std::string, uint64_t>(sensor_names[i], 0));
			deadlines_.insert(std::pair<std::string, uint64_t>(sensor_names[i], source_time_points[i]->get_deadline()));
			periods_.insert(std::pair<std::string, uint64_t>(sensor_names[i], source_time_points[i]->get_period()));
			source_time_points_.insert(std::pair<std::string, std::shared_ptr<SourceTimePoint>>(sensor_names[i], source_time_points[i]));
			#ifdef RECORD_LOG
			if(i == sensor_names.size()-1){
				ofs_<<"{\"sensor\":\""<<sensor_names[i]<<"\",\""<<"deadline\":"<<deadlines_[sensor_names[i]]<<",\"period\":"<<periods_[sensor_names[i]]<<"}],";
			}else{
				ofs_<<"{\"sensor\":\""<<sensor_names[i]<<"\",\""<<"deadline\":"<<deadlines_[sensor_names[i]]<<",\"period\":"<<periods_[sensor_names[i]]<<"},";//meta data of each sensor
			}
			#endif
		}
		#ifdef RECORD_LOG
		ofs_<<"\"logs\":[";
		#endif
		
	};
	~SinkTimePoint(){
		#ifdef RECORD_LOG
		std::cout<<"[Sink]log_dir_:"<<log_dir_<<std::endl;
		ofs_<<"]}";
		ofs_.close();
		#endif
	};
	void update_finish_times(std::map<std::string, TP_Info>&tp_infos){
		auto new_time = get_timestamp_in_ns();
		std::lock_guard<std::mutex> lock(mtx_);
		#ifdef RECORD_LOG
		if(tmp_log_!=""){
			tmp_log_=",";
		}
		#endif
		for(auto it = tp_infos.begin(); it != tp_infos.end(); it++){
			update_finish_time(it->first, new_time-it->second.last_sample_time_, it->second);
		}
		#ifdef RECORD_LOG
		tmp_log_.pop_back();//remove the last ','
		ofs_<<tmp_log_;
		#endif
	}
	
	private:
	//this function is the end of the pipeline, the log decode is done here
	void update_finish_time(const std::string& sensor_name, uint64_t new_time, TP_Info&tp_info, uint64_t update_ratio=30){
		auto it = finish_times_.find(sensor_name);
		if (it == finish_times_.end()){
			assert(false);
		}else{
			auto deadline = deadlines_[sensor_name];
			#ifdef RECORD_LOG
			tmp_log_ += "{\"sensor\":\""+sensor_name+"\","+tp_info.output_log()+"},";
			#endif
			//check whether a deadline miss happens
			if(new_time > deadline){
				#ifdef PRINT_DEBUG
				std::cout<<"[Sink]sensor:"<<sensor_name<<"'s pipeline costs:"<<new_time<<" reference time is:"<<it->second<<std::endl;
				std::cout<<"deadline miss happens"<<std::endl;
				#endif
				//do something
				source_time_points_[sensor_name]->set_remain_time(source_time_points_[sensor_name]->get_remain_time()*0.8);//make the pipeline tend to be speed first
				return;
			}
			//no deadline miss happens
			
			//first time
			if(it->second == 0){
				it->second = new_time;
				source_time_points_[sensor_name]->set_remain_time(deadline - new_time);
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
	std::map<std::string, uint64_t>deadlines_;//store the deadline of each sensor to avoid visiting the source time point
	std::map<std::string, uint64_t>periods_;//store the periods of each sensor to avoid visiting the source time point
	#ifdef RECORD_LOG
	std::string log_dir_;
	std::ofstream ofs_;
	std::string tmp_log_;
	#endif
	};
}
#endif  // INTERNEURON_LIB__SINK_TIME_POINT_HPP_