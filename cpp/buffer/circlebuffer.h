
#ifndef __LIVE_SCAN_CIRCLE_BUFFER_725629_H__
#define __LIVE_SCAN_CIRCLE_BUFFER_725629_H__

#include <boost/function.hpp>

namespace cosin
{

	class circle_buffer {
	public:
		typedef boost::function<int (const char *data, int offset, int length)> consumer_type;
	private:
		const int bufferSize;
		char *data;
		int beginPos;	// 数据开始位置
		int endPos;		// 数据结束位置的下一位置
		int dataSize;

	public:
		explicit circle_buffer(int bufSize);
		~circle_buffer();

		bool empty();

		bool full();

		int dataTotalSize();

		int freeSize();

		int write(const char *data, int offset, int length);

		int read(int length, consumer_type dataConsumer);

	private:
		void increaseBeginPos(int incremental);

		void increaseEndPos(int incremental);

		int increaseIndex(int x, int incremental, int m);
	};

}

#endif