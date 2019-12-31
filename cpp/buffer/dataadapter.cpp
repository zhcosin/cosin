
#include "dataadapter.h"

namespace cosin
{
	data_adapter::data_adapter(int bufSize, const consumer_type& consumer, int consume_mode)
		: runFlag(false), bufferSize(bufSize), dataQueue(bufSize), dataConsumer(consumer), consumeMode(consume_mode)
		, writeTimeout(2000), readTimeout(2000)
	{

	}

	void data_adapter::start()
	{
		{
			boost::mutex::scoped_lock lock(this->runFlagMutex);

			if (this->runFlag)
				return;

			this->runFlag = true;
		}

		this->consumeThread.reset(new boost::thread(&this_type::run, this));
	}

	int data_adapter::put(const char *data, int offset, int length) {
		if (data == NULL || offset < 0 || length <= 0)
			return 0;

		{
			boost::mutex::scoped_lock lock(this->runFlagMutex);

			if (!this->runFlag)
				return 0;
		}

		int writeSize = 0;

		{
			boost::mutex::scoped_lock lock(this->bufferMutex);

			do {
				while (dataQueue.full()) {
					this->cond_write.wait(lock);
				}

				int putSize = dataQueue.write(data, offset, length);

				offset += putSize;
				length -= putSize;

				this->cond_read.notify_one();

				if (putSize == 0)
					return writeSize;

				writeSize += putSize;
			} while (length > 0);

			return writeSize;
		}
	}

	void data_adapter::reset() {
		if (!dataQueue.empty()) {
			dataQueue.read(dataQueue.dataTotalSize(), dataConsumer);
		}
	}

	void data_adapter::stop() {
		{
			boost::mutex::scoped_lock lock(this->runFlagMutex);

			if (!this->runFlag)
				return;

			this->runFlag = false;
		}

		reset();
	}

	void data_adapter::run() {
		// TODO Auto-generated method stub
		while (true) {
			{
				boost::mutex::scoped_lock lock(this->runFlagMutex);

				if (!runFlag)
					return;
			}

			{
				boost::mutex::scoped_lock lock(this->bufferMutex);

				bool needRead = false;

				if (consumeMode == consumeWhenFull) {
					if (!dataQueue.full()) {
						this->cond_read.timed_wait(lock, boost::posix_time::milliseconds(this->readTimeout));
					}

					needRead = dataQueue.full();
				} else {
					if (dataQueue.empty()) 
						this->cond_read.timed_wait(lock, boost::posix_time::milliseconds(this->readTimeout));

					needRead = !dataQueue.empty();
				}

				if (needRead) {
					dataQueue.read(dataQueue.dataTotalSize(), dataConsumer);
					this->cond_write.notify_one();
				}
			}
		}
	}
}