
#include "timer.h"

#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/bind.hpp"

namespace cosin{

timer::task_item::task_item()
	: duration(0), accu_dur(0) {}
timer::task_item::task_item( const task_type &w, int dur )
	: work(w), duration(dur), accu_dur(0) {}


timer::timer( int dur, int thread_count )
: running(false), invoke_dur(dur > 50 ? dur : 50), work_thread_pool(thread_count > 1 ? thread_count : 1)
{

}

timer::~timer()
{
	if (this->is_running())
		this->close();
}

void timer::open()
{
	scoped_lock_type lock(this->my_mutex);

	if (this->running)
		return;

	this->running = true;

	this->loop_thread.reset(new boost::thread(boost::bind(&timer::loop, this)));
}

void timer::close()
{
	{
		scoped_lock_type lock(this->my_mutex);

		this->running = false;
	}

	this->cond_var.notify_one();

	this->loop_thread->timed_join(boost::posix_time::milliseconds(3 * this->invoke_dur));
	this->loop_thread.reset();

	this->work_thread_pool.clear();
}

bool timer::is_running() throw()
{
	scoped_lock_type lock(this->my_mutex);

	return this->running;
}

const std::string timer::add_task( const task_type &work, int dur )
{
	std::string id(boost::lexical_cast<std::string>(boost::uuids::random_generator()()));

	{
		scoped_lock_type lock(this->my_mutex);
		this->task_container[id] = task_item(work, (dur > this->invoke_dur ? dur : this->invoke_dur));
	}

	return id;
}

const std::string timer::add_task( const task_with_param_type &work, const boost::any& user_data, int dur )
{
	task_type the_task = boost::bind(work, user_data);

	return this->add_task(the_task, dur);
}

bool timer::remove_task( const std::string& task_id )
{
	scoped_lock_type lock(this->my_mutex);
	task_container_type::const_iterator it = this->task_container.find(task_id);

	if (this->task_container.end() == it)
	{
		return false;
	} 
	else
	{
		this->task_container.erase(it);
		return true;
	}
}

void timer::loop() throw()
{
	scoped_lock_type lock(this->my_mutex);

	boost::posix_time::time_duration exec_dur = boost::posix_time::microseconds(0);
	int sleep_dur = 0;

	while (this->running)
	{
		sleep_dur = this->invoke_dur * 1000 - exec_dur.total_microseconds();
		if (sleep_dur > 0)
			this->cond_var.timed_wait(lock, boost::posix_time::microseconds(sleep_dur));

		boost::posix_time::ptime t1(boost::posix_time::microsec_clock::local_time());
		this->do_work();
		boost::posix_time::ptime t2(boost::posix_time::microsec_clock::local_time());
		exec_dur = t2 - t1;
	}	
}

void timer::do_work() throw()
{
	for (task_container_type::iterator it = this->task_container.begin();
		it != this->task_container.end(); ++it)
	{
		task_item &the_task_item = it->second;
		the_task_item.accu_dur += this->invoke_dur;

		if (the_task_item.accu_dur >= the_task_item.duration)
		{
			the_task_item.accu_dur = 0;
			this->work_thread_pool.schedule(the_task_item.work);
		}
	}
}

}