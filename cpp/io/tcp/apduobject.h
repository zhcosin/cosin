
#ifndef __LIBAPDU_APDUOBJECT_H__
#define __LIBAPDU_APDUOBJECT_H__

#include "include.h"

namespace apdu{
	
	class apdu_object
	{
		const int handle;
	public:
		apdu_object();
	public:
		const int get_id() const;
	};

	class apdu_runner
	{
		mutable boost::mutex run_flag_mutex;
		bool run_flag;
	public:
		apdu_runner();
		virtual ~apdu_runner();

	public:
		void start();
		void stop();
		bool is_running() const;

	protected:
		virtual void on_start() = 0;
		virtual void on_stop() = 0;
	};

} // namespace apdu

#endif // __LIBAPDU_APDUOBJECT_H__
