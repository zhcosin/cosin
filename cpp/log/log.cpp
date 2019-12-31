
#include "log.h"

#include "boost/date_time/posix_time/posix_time.hpp"

namespace cosin{

log_collector::log_collector(const std::string& name) : name_(name) 
{

}

log_level::log_level(int level, const char *name) : level_(level), name_(name) 
{

}

log_level log_trace(0, "trace");
log_level log_info(1, "info");
log_level log_warning(2, "warning");
log_level log_error(3, "error");
log_level log_fatal(4, "fatal");

bool log_level_filter::filter(const log_level& level, const std::string& message)
{
	return level.level_ >= this->level_.level_;
}

bool log_all_print::filter(const log_level& level, const std::string& message)
{
	return true;
}

bool log_all_no_print::filter(const log_level& level, const std::string& message)
{
	return false;
}

void log_to_console::save(const log_level& lv, const std::string& message)
{
	boost::mutex::scoped_lock console_lock(console_mutex);
	if (lv.level_ >= log_error.level_)
	{
		std::cerr << message << std::endl;
	}
	else
	{
		std::cout << message << std::endl;
	}
}

log_to_file::log_to_file(const std::string& filename) : ofs(filename.c_str())
{

}

void log_to_file::save(const log_level& lv, const std::string& message)
{
	boost::mutex::scoped_lock file_lock(file_mutex);
	ofs << message << "\n";
}

void log_to_blackhole::save(const log_level& lv, const std::string& message)
{
}

log_to_cb::log_to_cb(log_callback cb) : cb_(cb) 
{

}

void log_to_cb::save(const log_level& lv, const std::string& message)
{
	boost::mutex::scoped_lock cb_lock(cb_mutex);
	cb_(lv.level_, message.c_str());
}

logger::logger() 
{

}

// 输出日志
void logger::log(const log_collector& collector, const log_level& level, const std::string& message) const
{
	// 获取当前时间并转换为字符串
	boost::posix_time::ptime now(boost::posix_time::microsec_clock::local_time());
	std::string now_str = boost::posix_time::to_iso_extended_string(now);
	boost::replace_first(now_str, "T", " ");

	std::stringstream ss;
	ss << "<" << now_str << "> " 
		<< "[" << level.name_ << "] " 
		<< "[" << collector.name_ << "]\n"
		<< message << "\n";

	for (int i = 0; i < backendList.size(); ++i)
	{
		boost::shared_ptr<log_backend> backend = backendList[i];
		backend->save(level, ss.str());
	}
}

logger& logger::instance()
{
	static logger the_logger;
	return the_logger;
}

void logger::add_backend( boost::shared_ptr<log_backend> backend )
{
	this->backendList.push_back(backend);
}

log_backend::log_backend(boost::shared_ptr<log_filter> thefilter, boost::shared_ptr<log_destination> thedestination)
: filter(thefilter), destination(thedestination)
{

}

boost::shared_ptr<log_filter> log_backend::getFilter() const
{
	return this->filter;
}

boost::shared_ptr<log_destination> log_backend::getDestination() const
{
	return this->destination;
}

void log_backend::save( const log_level& lv, const std::string& message )
{
	if (this->filter->filter(lv, message)) {
		this->destination->save(lv, message);
	}
}

}