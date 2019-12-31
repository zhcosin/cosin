
#ifndef __COSIN_TIMER_H_
#define __COSIN_TIMER_H_

#include <string>
#include <map>
#include <stdexcept>

#include "boost/shared_ptr.hpp"
#include "boost/any.hpp"
#include "boost/function.hpp"
#include "boost/thread.hpp"
#include "boost/thread/mutex.hpp"
#include "threadpool/threadpool.hpp"

namespace cosin{

	class timer
	{
	public:
		typedef boost::function<void ()> task_type;
		typedef boost::function<void (const boost::any&)> task_with_param_type;

	private:
		struct task_item
		{
			task_type work;
			int duration;
			int accu_dur;

			task_item();
			task_item(const task_type &w, int dur);
		};

		typedef boost::mutex	locker_type;
		typedef boost::mutex::scoped_lock	scoped_lock_type;

		typedef boost::shared_ptr<boost::thread> thread_ptr;

		typedef std::map<std::string, task_item>	task_container_type;

		int invoke_dur;

		task_container_type	task_container;

		locker_type	my_mutex;
		bool running;

		thread_ptr loop_thread;
		boost::condition_variable cond_var;

		boost::threadpool::pool work_thread_pool;

		void loop() throw();
		void do_work() throw();

	public:
		timer(int dur, int thread_count);
		~timer();

		void open();
		void close();

		bool is_running() throw();

		const std::string add_task(const task_type &work, int dur);
		const std::string add_task(const task_with_param_type &work, const boost::any& user_data, int dur);
		bool remove_task(const std::string& task_id);
	};

}

#endif
