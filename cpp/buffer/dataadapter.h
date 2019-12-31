
#ifndef __LIVE_SCAN__DATA_APDATER_736739_H__
#define __LIVE_SCAN__DATA_APDATER_736739_H__

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "circlebuffer.h"

namespace cosin
{
	class data_adapter {
	public:
		typedef data_adapter this_type;
		typedef circle_buffer::consumer_type consumer_type;
	private:
		const int bufferSize;
		boost::mutex bufferMutex;
		circle_buffer dataQueue;
		boost::shared_ptr<boost::thread> consumeThread;
		consumer_type dataConsumer;
		boost::condition_variable cond_write;
		boost::condition_variable cond_read;
		boost::mutex runFlagMutex;
		bool runFlag;
		int writeTimeout;		// 写数据时等待信号量超时时长
		int readTimeout;		// 读数据时等待信号量超时时长
		int consumeMode;
		
	public:
		static const int consumeWhenFull = 0;
		static const int consumeWhenNotEmpty = 1;

	public:
		data_adapter(int bufSize, const consumer_type& consumer, int consume_mode);

	public:
		void start();

		void stop();

		int put(const char *data, int offset, int length);

	private:
		void reset();
		void run();
	};

}

#endif