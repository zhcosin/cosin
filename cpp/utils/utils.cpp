
#include <sstream>

#include <boost/thread.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/random.hpp>

#include "utils.h"

namespace cosin
{


std::string utils::getCurrentThreadId()
{
	boost::thread::id threadId = boost::this_thread::get_id();

	static std::stringstream ss;

	ss.clear();
	ss << threadId;

	return ss.str();
}

std::string utils::time2SampleString( const boost::posix_time::ptime& time)
{
	if (time.is_not_a_date_time())
		return "";

	std::string str = boost::posix_time::to_iso_extended_string(time);
	boost::algorithm::replace_all(str, ":", "-");

	return str;
}

int utils::random_number( int min, int max )
{
	static boost::mt19937 rng(time(0));

	if (min < max)
	{
		boost::uniform_int<> ui(min, max);
		return ui(rng);
	}
	else if (min == max)
	{
		return min;
	}
	else
	{
		return 0;
	}
}

}
