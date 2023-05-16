/*
 * @Description: 
 * @Author: Sauron
 * @Date: 2023-03-06 22:45:22
 * @LastEditTime: 2023-03-08 11:41:31
 * @LastEditors: Sauron
 */
#include "trace_picas/trace.hpp"

namespace trace {
    
    Trace::Trace(const std::string fname) : file_name_(fname) {
        file_.open(file_name_,std::ofstream::out | std::ofstream::trunc);
    }

    Trace::~Trace() {
        file_.close();
    }

    void Trace::trace_write(const std::string index, std::string value) {
            file_ << index << " " << value << std::endl;
    }

    void Trace::trace_write_count(const std::string index, std::string value, std::string count) {
            file_ << index << " " << value << " " << count << std::endl;
    }

}
