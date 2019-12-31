
#ifndef __LIVE_SCAN_LOG_383437_H__
#define __LIVE_SCAN_LOG_383437_H__

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>

namespace cosin{
	
	// 负责搜集日志，含有日志标签等信息
	class log_collector
	{
	public:
		const std::string name_;
	public:
		explicit log_collector(const std::string& name);
	};

	class log_level
	{
	public:
		const int level_;
		const std::string name_;
	public:
		log_level(int level, const char *name);
	};

	extern log_level log_trace;
	extern log_level log_info;
	extern log_level log_warning;
	extern log_level log_error;
	extern log_level log_fatal;

	class log_filter
	{
	public:
		virtual bool filter(const log_level& level, const std::string& message) = 0;
	};

	// 按等级过滤日志，大于等于指定等级的日志可以通过
	struct log_level_filter : public log_filter
	{
		log_level level_;
		explicit log_level_filter(const log_level& level);

		bool filter(const log_level& level, const std::string& message);
	};

	// 默认日志过滤器，不拦截任何日志
	class log_all_print : public log_filter
	{
	public:
		bool filter(const log_level& level, const std::string& message);
	};

	// 拦截全部日志
	class log_all_no_print : public log_filter
	{
	public:
		bool filter(const log_level& level, const std::string& message);
	};

	// 日志输出地
	class log_destination
	{
	public:
		virtual void save(const log_level& lv, const std::string& message) = 0;
	};

	// 默认日志输出地，控制台屏幕
	class log_to_console : public log_destination
	{
		boost::mutex console_mutex;
	public:
		void save(const log_level& lv, const std::string& message);
	};

	// 输出到文件
	class log_to_file : public log_destination
	{
		boost::mutex file_mutex;
		std::ofstream ofs;
	public:
		explicit log_to_file(const std::string& filename);
	public:
		void save(const log_level& lv, const std::string& message);
	};

	// 黑洞，直接丢弃
	class log_to_blackhole : public log_destination
	{
	public:
		void save(const log_level& lv, const std::string& message);
	};

	// 接收日志的回调函数 
	 typedef void (*log_callback)(int level, const char * message);

	 class log_to_cb : public log_destination
	 {
		 boost::mutex cb_mutex;
		 log_callback cb_;
	 public:
		 explicit log_to_cb(log_callback cb);
	 public:
		 void save(const log_level& lv, const std::string& message);
	 };

	 // 日志后端，组合一个过滤器和一个输出地.
	 class log_backend
	 {
	 private:
		 boost::shared_ptr<log_filter> filter;
		 boost::shared_ptr<log_destination> destination;

	 public:
		 log_backend(boost::shared_ptr<log_filter> thefilter, boost::shared_ptr<log_destination> thedestination);

	 public:
		boost::shared_ptr<log_filter> getFilter() const;
		boost::shared_ptr<log_destination> getDestination() const;

	 public:
		void save(const log_level& lv, const std::string& message);
	 };

	 // 日志管理器
	 class logger
	 {
		 std::vector< boost::shared_ptr<log_backend> > backendList;

	 private:
		 logger();

	 public:

		 // 获取唯一实例
		 static logger& instance();

		 // 添加日志后端
		 void add_backend(boost::shared_ptr<log_backend> backend);

		 // 输出日志
		 void log(const log_collector& collector, const log_level& level, const std::string& message) const;
	 };
}

#endif // __LIVE_SCAN_LOG_383437_H__