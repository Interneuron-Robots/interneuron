/*
 * @Description:
 * @Author: Sauron
 * @Date: 2023-05-10 16:33:52
 * @LastEditTime: 2023-07-21 14:49:23
 * @LastEditors: Sauron
 */

#ifndef INTERNEURON_LIB__INTERNEURON_MONITOR_HPP_
#define INTERNEURON_LIB__INTERNEURON_MONITOR_HPP_
//#include "interneuron_msgs/msg/interneuron_header.hpp"
#include <vector>
#include <string>
#include "time_point.hpp"
#include "source_time_point.hpp"
#include "middle_time_point.hpp"
#include "sink_time_point.hpp"
#include "visibility_control.hpp"

namespace interneuron
{

	//for msgs from different channels
	bool fusion_msgs(std::map<std::string, interneuron::TP_Info>&m0, std::map<std::string, interneuron::TP_Info>&m1, std::map<std::string, interneuron::TP_Info>&result, FusionPolicy policy);
	class TimePointManager
	{
	public:
		// Static access method. This function is how we access the singleton object.
		static TimePointManager &getInstance()
		{
			static TimePointManager instance; // Guaranteed to be destroyed.
							  // Instantiated on first use.
			return instance;
		}

		// Delete copy constructor and assignment operator.
		// This is important, because we don't want to accidentally make copies of the singleton.
		TimePointManager(const TimePointManager &) = delete;		// Copy constructor
		TimePointManager &operator=(const TimePointManager &) = delete; // Copy assignment operator
		TimePointManager(TimePointManager &&) = delete;			// Move constructor
		TimePointManager &operator=(TimePointManager &&) = delete;	// Move assignment operator

		INTERNEURON_PUBLIC
		std::shared_ptr<SourceTimePoint> add_source_timepoint(const std::string &sensor_name, uint64_t deadline, uint64_t period);
		INTERNEURON_PUBLIC
		std::shared_ptr<MiddleTimePoint> add_middle_timepoint(const std::string &key, const std::vector<std::string> &sensor_names);
		INTERNEURON_PUBLIC
		std::shared_ptr<SinkTimePoint> add_sink_timepoint(const std::string &key, const std::vector<std::string> &sensor_names);
		
// we cannot use message_info directly, so it's rmw, rcl and rclcpp's job to provide needed information
		INTERNEURON_PUBLIC
		std::shared_ptr<TimePoint> get_timepoint(const std::string &key, TimePointType type);


	private:
		// Private constructor so that no objects can be created.
		TimePointManager()
		{
			// Initialization of the singleton, if needed
		}
		std::mutex source_mtx_;
		std::mutex middle_mtx_;
		std::mutex sink_mtx_;
		std::map<std::string, std::shared_ptr<SourceTimePoint>> source_time_points_;// key is the sub/pub_id + "_source"
		std::map<std::string, std::shared_ptr<MiddleTimePoint>> middle_time_points_;// key is the sub/pub_id + "_sub/_pub/_app"
		std::map<std::string, std::shared_ptr<SinkTimePoint>> sink_time_points_;// key is the sub/pub_id + "_sink"

	};
}
#endif