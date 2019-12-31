
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
	
	// �����Ѽ���־��������־��ǩ����Ϣ
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

	// ���ȼ�������־�����ڵ���ָ���ȼ�����־����ͨ��
	struct log_level_filter : public log_filter
	{
		log_level level_;
		explicit log_level_filter(const log_level& level);

		bool filter(const log_level& level, const std::string& message);
	};

	// Ĭ����־���������������κ���־
	class log_all_print : public log_filter
	{
	public:
		bool filter(const log_level& level, const std::string& message);
	};

	// ����ȫ����־
	class log_all_no_print : public log_filter
	{
	public:
		bool filter(const log_level& level, const std::string& message);
	};

	// ��־�����
	class log_destination
	{
	public:
		virtual void save(const log_level& lv, const std::string& message) = 0;
	};

	// Ĭ����־����أ�����̨��Ļ
	class log_to_console : public log_destination
	{
		boost::mutex console_mutex;
	public:
		void save(const log_level& lv, const std::string& message);
	};

	// ������ļ�
	class log_to_file : public log_destination
	{
		boost::mutex file_mutex;
		std::ofstream ofs;
	public:
		explicit log_to_file(const std::string& filename);
	public:
		void save(const log_level& lv, const std::string& message);
	};

	// �ڶ���ֱ�Ӷ���
	class log_to_blackhole : public log_destination
	{
	public:
		void save(const log_level& lv, const std::string& message);
	};

	// ������־�Ļص����� 
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

	 // ��־��ˣ����һ����������һ�������.
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

	 // ��־������
	 class logger
	 {
		 std::vector< boost::shared_ptr<log_backend> > backendList;

	 private:
		 logger();

	 public:

		 // ��ȡΨһʵ��
		 static logger& instance();

		 // �����־���
		 void add_backend(boost::shared_ptr<log_backend> backend);

		 // �����־
		 void log(const log_collector& collector, const log_level& level, const std::string& message) const;
	 };
}

#endif // __LIVE_SCAN_LOG_383437_H__