
#include "include.h"
#include "apduobject.h"
#include "utils/utils.h"

namespace apdu{

	apdu_object::apdu_object() : handle(utils::random_number(1, 1000000)) {}

	const int apdu_object::get_id() const
	{
		return this->handle;
	}

	apdu_runner::apdu_runner() : run_flag(false)
	{

	}

	apdu_runner::~apdu_runner()
	{

	}

	void apdu_runner::start()
	{
		{
			boost::mutex::scoped_lock lock(this->run_flag_mutex);
			this->run_flag = true;
		}

		this->on_start();
	}

	void apdu_runner::stop()
	{
		this->on_stop();

		{
			boost::mutex::scoped_lock lock(this->run_flag_mutex);
			this->run_flag = false;
		}
	}

	bool apdu_runner::is_running() const
	{
		boost::mutex::scoped_lock lock(this->run_flag_mutex);
		return this->run_flag;
	}

}
