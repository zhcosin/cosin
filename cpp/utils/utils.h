#ifndef __LIVE_SCAN_UTILS_s4727_H__
#define __LIVE_SCAN_UTILS_s4727_H__

#include <string>

#include <boost/date_time/posix_time/posix_time.hpp>

namespace cosin
{


class utils
{
public:
	static std::string getCurrentThreadId();

	static int random_number(int min, int max);

	static std::string time2SampleString( const boost::posix_time::ptime& time);
};

}

#endif // __LIVE_SCAN_UTILS_s4727_H__