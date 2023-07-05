#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <sys/time.h>

// For ROS2RTF
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/syscall.h>
#include <mutex>

#include <iostream>
#include "trace_picas/trace.hpp"

#include "rclcpp/rclcpp.hpp"
// #include "std_msgs/msg/string.hpp"
#include "test_msgs/msg/test_string.hpp"
// #include "test_msgs/msg/detail/test_string__struct.hpp"
#ifdef INTERNEURON
#include "interneuron_lib/time_point_manager.hpp"
using std::placeholders::_2;
#endif
using std::placeholders::_1;
// std::mutex mtx;
#include "fastrtps/log/StdoutConsumer.h"
#include "fastrtps/log/Log.h"

#define gettid() syscall(__NR_gettid)

// #define USE_INTRA_PROCESS_COMMS false
#define USE_INTRA_PROCESS_COMMS true

#define DUMMY_LOAD_ITER 100
int dummy_load_calib = 1;

void dummy_load(int load_ms)
{
    int i, j;
    for (j = 0; j < dummy_load_calib * load_ms; j++)
        for (i = 0; i < DUMMY_LOAD_ITER; i++)
            __asm__ volatile("nop");
}

using namespace std::chrono_literals;

class StartNode : public rclcpp::Node
{
public:
    StartNode(const std::string node_name, const std::string pub_topic, std::shared_ptr<trace::Trace> trace_ptr, int exe_time, int period, bool end_flag)
        : Node(node_name, rclcpp::NodeOptions().use_intra_process_comms(USE_INTRA_PROCESS_COMMS)), count_(0), trace_callbacks_(trace_ptr), exe_time_(exe_time), period_(period), end_flag_(end_flag)
    {
        // publisher_ = this->create_publisher<std_msgs::msg::String>(pub_topic, 1);
        publisher_ = this->create_publisher<test_msgs::msg::TestString>(pub_topic, 1);

        if (period_ == 1000)
            timer_ = this->create_wall_timer(1000ms, std::bind(&StartNode::timer_callback, this));
        else
            timer_ = this->create_wall_timer(500ms, std::bind(&StartNode::timer_callback, this));

        gettimeofday(&create_timer, NULL);
        RCLCPP_INFO(this->get_logger(), "Create wall timer at %ld", create_timer.tv_sec * 1000 + create_timer.tv_usec / 1000);
#ifdef INTERNEURON
        // StartNode only has one timepoint for publisher
        interneuron::TimePointManager::getInstance().add_timepoint(publisher_->get_key_tp()+"sen", {"c1_sensor"});
#endif
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<test_msgs::msg::TestString>::SharedPtr publisher_;

private:
    size_t count_;
    int exe_time_;
    int period_;
    timeval ctime, ftime, create_timer, latency_time;
    bool end_flag_;
    std::shared_ptr<trace::Trace> trace_callbacks_;

    void timer_callback()
    {
        std::string name = this->get_name();
        RCLCPP_INFO(this->get_logger(), ("callback: " + name).c_str());
        gettimeofday(&ctime, NULL);
        // trace_callbacks_->trace_write(name+"_in",std::to_string(ctime.tv_sec*1000+ctime.tv_usec/1000));
        // trace_callbacks_->trace_write_count(name+"_in",std::to_string(ctime.tv_sec*1000+ctime.tv_usec/1000),std::to_string(count_));

        dummy_load(exe_time_);

        auto message = test_msgs::msg::TestString();
        message.data = std::to_string(count_++);
        message.stamp.sec = ctime.tv_sec;
        message.stamp.usec = ctime.tv_usec;

        // gettimeofday(&ftime, NULL);
        // trace_callbacks_->trace_write(name+"_out",std::to_string(ftime.tv_sec*1000+ftime.tv_usec/1000));
        if (end_flag_)
        {
            gettimeofday(&ftime, NULL);
            latency_time.tv_sec = (ftime.tv_sec - message.stamp.sec);
            latency_time.tv_usec = (ftime.tv_usec - message.stamp.usec);
            // trace_callbacks_->trace_write_count(name+"_latency",std::to_string(latency_time.tv_sec*1000000+latency_time.tv_usec), message.data);
        }

#ifdef INTERNEURON
        auto message_info = std::make_unique<rclcpp::MessageInfo>(rclcpp::MessageInfo());
        message_info->this_sample_time_ = static_cast<uint64_t>(ctime.tv_sec * 1000000 + ctime.tv_usec);

        auto tp = interneuron::TimePointManager::getInstance().get_timepoint(publisher_->get_key_tp()+"sen");
        interneuron::TimePointManager::getInstance().set_deadline(2300000);
        auto remain_time = interneuron::TimePointManager::getInstance().get_remain_time();// if different sensor has different deadlines, we cannot use a single remain_time
        remain_time = remain_time != 0 ? remain_time : 100*1000;// for the first time, we set remain_time to be 100 ms
        std::cout<<"remain_time: "<<remain_time<<std::endl;
        message_info->remain_time_ = remain_time;

        tp->lock();
        auto old_sample_time = tp->update_last_sample_time(message_info->this_sample_time_);
        message_info->last_sample_time_ = old_sample_time == 0 ? message_info->this_sample_time_ - period_*1000 : old_sample_time;
        auto sample_time = message_info->this_sample_time_ - message_info->last_sample_time_;
        auto policy = tp->update_reference_time("c1_sensor", sample_time, remain_time); // remain_time should change according to previous pipeline's finish time and deadline
        std::cout<<this->get_name()<<"'s sample reference time: "<<tp->reference_times_["c1_sensor"]<<", and sample time: "<<sample_time<<std::endl;
        tp->unlock();

        switch(policy){
            case interneuron::Policy::QualityFirst:
            std::cout<<"QualityFirst"<<std::endl;
            break;
            case interneuron::Policy::SpeedFirst:
            std::cout<<"SpeedFirst"<<std::endl;
            break;
            case interneuron::Policy::Emergency:
            std::cout<<"Emergency"<<std::endl;
            break;
            case interneuron::Policy::Error:
            std::cout<<"Error"<<std::endl;
            break;
            default:
            std::cout<<"Unknown"<<std::endl;
        }
        publisher_->publish(message, std::move(message_info));
#else
        publisher_->publish(message);
#endif
    }
};

class IntermediateNode : public rclcpp::Node
{
public:
    IntermediateNode(const std::string node_name, const std::string sub_topic, const std::string pub_topic, std::shared_ptr<trace::Trace> trace_ptr, int exe_time, bool end_flag)
        : Node(node_name, rclcpp::NodeOptions().use_intra_process_comms(USE_INTRA_PROCESS_COMMS)), count_(0), trace_callbacks_(trace_ptr), exe_time_(exe_time), end_flag_(end_flag)
    {
        if (pub_topic != ""){
            publisher_ = this->create_publisher<test_msgs::msg::TestString>(pub_topic, 1);
        }
#ifdef INTERNEURON
        subscription_ = this->create_subscription<test_msgs::msg::TestString>(sub_topic, 10, std::bind(&IntermediateNode::callback, this, _1, _2));
        interneuron::TimePointManager::getInstance().add_timepoint(subscription_->get_key_tp()+"sub", {"c1_sensor"});// for ros2
        interneuron::TimePointManager::getInstance().add_timepoint(subscription_->get_key_tp()+"app", {"c1_sensor"});
        if(publisher_!=nullptr){
            interneuron::TimePointManager::getInstance().add_timepoint(publisher_->get_key_tp()+"pub", {"c1_sensor"});
        }
#else
        subscription_ = this->create_subscription<test_msgs::msg::TestString>(sub_topic, 1, std::bind(&IntermediateNode::callback, this, _1));
#endif
    }

    rclcpp::Publisher<test_msgs::msg::TestString>::SharedPtr publisher_ = nullptr;
    rclcpp::Subscription<test_msgs::msg::TestString>::SharedPtr subscription_;

private:
    size_t count_;
    int exe_time_;
    timeval ctime, ftime, latency_time;
    double latency;
    std::shared_ptr<trace::Trace> trace_callbacks_;
    bool end_flag_;
#ifdef INTERNEURON
    void callback(const test_msgs::msg::TestString::SharedPtr msg, const rclcpp::MessageInfo& msg_info)
    {
        gettimeofday(&ctime, NULL);
        // interneuron::TimePointManager::getInstance().default_update("c1", "Regular_callback1");
        auto run_start = ctime.tv_sec * 1000000 + ctime.tv_usec;
        auto receive_time = run_start - msg_info.last_sample_time_;
        auto tp = interneuron::TimePointManager::getInstance().get_timepoint(subscription_->get_key_tp()+"app");
        std::cout<<this->get_name()<<"'s receive reference time: "<<tp->reference_times_["c1_sensor"]<<", and receive time: "<<receive_time<<std::endl;
        tp->lock();
        auto policy = tp->update_reference_time("c1_sensor",  receive_time, msg_info.remain_time_); // we use the period as the remain_time
        tp->unlock();
        switch(policy){
            case interneuron::Policy::QualityFirst:
            std::cout<<"QualityFirst"<<std::endl;
            break;
            case interneuron::Policy::SpeedFirst:
            std::cout<<"SpeedFirst"<<std::endl;
            break;
            case interneuron::Policy::Emergency:
            std::cout<<"Emergency"<<std::endl;
            break;
            case interneuron::Policy::Error:
            std::cout<<"Error"<<std::endl;
            break;
            default:
            std::cout<<"Unknown"<<std::endl;
        }
        std::string name = this->get_name();
        RCLCPP_INFO(this->get_logger(), ("callback: " + name).c_str());
        // gettimeofday(&ctime, NULL);
        // trace_callbacks_->trace_write(name+"_in",std::to_string(ctime.tv_sec*1000+ctime.tv_usec/1000));
        dummy_load(exe_time_);

        auto message = test_msgs::msg::TestString();
        message.data = msg->data;
        message.stamp = msg->stamp;

        if (end_flag_)
        {
            gettimeofday(&ftime, NULL);
            latency_time.tv_sec = (ftime.tv_sec - msg->stamp.sec);
            latency_time.tv_usec = (ftime.tv_usec - msg->stamp.usec);
            trace_callbacks_->trace_write_count(name + "_latency", std::to_string(latency_time.tv_sec * 1000000 + latency_time.tv_usec), message.data);
        }

        gettimeofday(&ctime, NULL);
        auto pub_time = ctime.tv_sec * 1000000 + ctime.tv_usec;
        auto finish_time = pub_time - msg_info.last_sample_time_;
        if (publisher_!=nullptr){
        tp = interneuron::TimePointManager::getInstance().get_timepoint(publisher_->get_key_tp()+"pub");
        tp->lock();
        std::cout<<this->get_name()<<"'s finish reference time: "<<tp->reference_times_["c1_sensor"]<<", and finish time: "<<finish_time<<std::endl;
        policy = tp->update_reference_time("c1_sensor",  finish_time, msg_info.remain_time_); 
        tp->unlock();
        switch(policy){
            case interneuron::Policy::QualityFirst:
            std::cout<<"QualityFirst"<<std::endl;
            break;
            case interneuron::Policy::SpeedFirst:
            std::cout<<"SpeedFirst"<<std::endl;
            break;
            case interneuron::Policy::Emergency:
            std::cout<<"Emergency"<<std::endl;
            break;
            case interneuron::Policy::Error:
            std::cout<<"Error"<<std::endl;
            break;
            default:
            std::cout<<"Unknown"<<std::endl;
        }
        auto message_info = std::make_unique<rclcpp::MessageInfo>(msg_info);
            publisher_->publish(message, std::move(message_info));//no need to change msg_info
        }else{
            if(interneuron::TimePointManager::getInstance().update_remain_time(finish_time)){
                std::cout<<"do something to handle the deadline miss"<<std::endl;
            }else{
                std::cout<<"finish within deadline, and the new remain_time is:"<<interneuron::TimePointManager::getInstance().get_remain_time()<<std::endl;
            }
        }
    }
#else
    void callback(const test_msgs::msg::TestString::SharedPtr msg)
    {
        std::string name = this->get_name();
        RCLCPP_INFO(this->get_logger(), ("callback: " + name).c_str());
        // gettimeofday(&ctime, NULL);
        // trace_callbacks_->trace_write(name+"_in",std::to_string(ctime.tv_sec*1000+ctime.tv_usec/1000));
        dummy_load(exe_time_);

        auto message = test_msgs::msg::TestString();
        message.data = msg->data;
        message.stamp = msg->stamp;

        if (end_flag_)
        {
            gettimeofday(&ftime, NULL);
            latency_time.tv_sec = (ftime.tv_sec - msg->stamp.sec);
            latency_time.tv_usec = (ftime.tv_usec - msg->stamp.usec);
            trace_callbacks_->trace_write_count(name + "_latency", std::to_string(latency_time.tv_sec * 1000000 + latency_time.tv_usec), message.data);
        }

        if (publisher_)
            publisher_->publish(message);
    }
#endif
};

int main(int argc, char *argv[])
{
    // Create a StdoutConsumer to output logs to the console
    eprosima::fastrtps::StdoutConsumer stdout_consumer;

    // Register the StdoutConsumer with the Log system
    eprosima::fastrtps::Log::RegisterConsumer(std::unique_ptr<eprosima::fastrtps::LogConsumer>(&stdout_consumer));

    // Set the log verbosity level
    eprosima::fastrtps::Log::SetVerbosity(eprosima::fastrtps::Log::Kind::Info);

    rclcpp::init(argc, argv);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "PID: %ld run in ROS2.", gettid());
    const char *rmw_implementation = rmw_get_implementation_identifier();
    RCLCPP_INFO(rclcpp::get_logger("middleware_check"), "Using RMW implementation: %s", rmw_implementation);

    // Naive way to calibrate dummy workload for current system
    while (1)
    {
        timeval ctime, ftime;
        int duration_us;
        gettimeofday(&ctime, NULL);
        dummy_load(100); // 50ms
        gettimeofday(&ftime, NULL);
        duration_us = (ftime.tv_sec - ctime.tv_sec) * 1000000 + (ftime.tv_usec - ctime.tv_usec);
        // RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "dummy_load_calib: %d (duration_us: %d ns)", dummy_load_calib, duration_us);
        if (abs(duration_us - 100 * 1000) < 500)
        { // error margin: 500us
            break;
        }
        dummy_load_calib = 100 * 1000 * dummy_load_calib / duration_us;
        if (dummy_load_calib <= 0)
            dummy_load_calib = 1;
    }
    timeval ctime;
    gettimeofday(&ctime, NULL);
    std::shared_ptr<trace::Trace> trace_callbacks = std::make_shared<trace::Trace>("data/trace.txt");
    trace_callbacks->trace_write("init", std::to_string(ctime.tv_sec * 1000 + ctime.tv_usec / 1000));

    // Create callbacks, exec_time and period are in ms
    auto c1_t_cb = std::make_shared<StartNode>("Timer_callback", "c1", trace_callbacks, 300, 1000, false);
    auto c1_r_cb_1 = std::make_shared<IntermediateNode>("Regular_callback1", "c1", "c2", trace_callbacks, 400, true);
    auto c1_r_cb_2 = std::make_shared<IntermediateNode>("Regular_callback2", "c2", "", trace_callbacks, 500, true);

    // auto c1_r_cb_2 = std::make_shared<IntermediateNode>("Regular_callback2", "c1", "", trace_callbacks, 1000, true);
    // auto c1_r_cb_3 = std::make_shared<IntermediateNode>("Regular_callback3", "c1", "", trace_callbacks, 1000, true);
    // auto c1_r_cb_1 = std::make_shared<IntermediateNode>("Regular_callback1", "c1", "c2", trace_callbacks, 1000, true);
    // auto c1_r_cb_2 = std::make_shared<IntermediateNode>("Regular_callback2", "c2", "c3", trace_callbacks, 1000, true);
    // auto c1_r_cb_3 = std::make_shared<IntermediateNode>("Regular_callback3", "c3", "c4", trace_callbacks, 1000, true);

    // Create executors
    rclcpp::executors::MultiThreadedExecutor exec1;

#ifdef PICAS
    // Enable priority-based callback scheduling
    exec1.enable_callback_priority();
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "PiCAS priority-based callback scheduling: %s", exec1.callback_priority_enabled ? "Enabled" : "Disabled");

    // Set executor's RT priority and CPU allocation
    exec1.set_executor_priority_cpu(90, 5);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "PiCAS executor 1's rt-priority %d and CPU %d", exec1.executor_priority, exec1.executor_cpu);
#endif

    // Allocate callbacks to executors
    exec1.add_node(c1_t_cb);
    exec1.add_node(c1_r_cb_1);
    exec1.add_node(c1_r_cb_2);
    // exec1.add_node(c1_r_cb_3);

#ifdef PICAS
    // Assign callbacks' priority
    exec1.set_callback_priority(c1_t_cb->timer_, 10);
    exec1.set_callback_priority(c1_r_cb_1->subscription_, 11);
    exec1.set_callback_priority(c1_r_cb_2->subscription_, 12);
    exec1.set_callback_priority(c1_r_cb_3->subscription_, 13);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Timer_callback->priority: %d", c1_t_cb->timer_->callback_priority);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Regular_callback1->priority: %d", c1_r_cb_1->subscription_->callback_priority);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Regular_callback2->priority: %d", c1_r_cb_2->subscription_->callback_priority);
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Regular_callback3->priority: %d", c1_r_cb_3->subscription_->callback_priority);

    std::thread spinThread1(&rclcpp::executors::SingleThreadedExecutor::spin_rt, &exec1);
#else
    std::thread spinThread1(&rclcpp::executors::MultiThreadedExecutor::spin, &exec1);
#endif

    spinThread1.join();

    exec1.remove_node(c1_t_cb);
    exec1.remove_node(c1_r_cb_1);
    // exec1.remove_node(c1_r_cb_2);
    // exec1.remove_node(c1_r_cb_3);

    rclcpp::shutdown();
    RCLCPP_INFO(rclcpp::get_logger("rclcpp"), "Close successfully");
    eprosima::fastrtps::Log::ClearConsumers();
    return 0;
}
