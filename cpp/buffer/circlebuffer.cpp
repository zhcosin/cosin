
#include "circlebuffer.h"

namespace cosin
{
	circle_buffer::circle_buffer(int bufSize) : bufferSize(bufSize), beginPos(0), endPos(0), dataSize(0) {
		this->data = new char[bufSize];
	}

	circle_buffer::~circle_buffer()
	{
		delete[] this->data;
	}

	bool circle_buffer::empty() {
		return dataSize == 0;
	}

	bool circle_buffer::full() {
		return dataSize == bufferSize;
	}

	int circle_buffer::dataTotalSize() {
		return dataSize;
	}

	int circle_buffer::freeSize() {
		return bufferSize - dataSize;
	}

	int circle_buffer::write(const char *data, int offset, int length) {
		if (data == NULL || offset < 0 || length <= 0)
			return 0;

		if (full())
			return 0;

		int remainderSize = this->bufferSize - this->dataSize;
		int writeSize = length < remainderSize ? length : remainderSize;

		int beginPutIndex = increaseIndex(this->endPos, 0, this->bufferSize);
		int destIndex, srcIndex;
		for (int i = 0; i < writeSize; ++i) {
			destIndex = increaseIndex(beginPutIndex, i, this->bufferSize);
			srcIndex = offset + i;
			this->data[destIndex] = data[srcIndex];
		}

		increaseEndPos(writeSize);
		this->dataSize += writeSize;

		return writeSize;
	}

	int circle_buffer::read(int length, consumer_type dataConsumer) {
		if (empty())
			return 0;

		int readSize = length < this->dataSize ? length : this->dataSize;
		if (this->endPos >= this->beginPos) {	// 数据只有一段
			dataConsumer(data, beginPos, readSize);
		} else {	// 数据分前后两段
			dataConsumer(data, beginPos, bufferSize - beginPos);
			dataConsumer(data, 0, endPos);
		}

		increaseBeginPos(readSize);
		this->dataSize -= readSize;

		return readSize;
	}

	void circle_buffer::increaseBeginPos(int incremental) {
		this->beginPos = increaseIndex(this->beginPos, incremental, this->bufferSize);
	}

	void circle_buffer::increaseEndPos(int incremental) {
		this->endPos = increaseIndex(this->endPos, incremental, this->bufferSize);
	}

	int circle_buffer::increaseIndex(int x, int incremental, int m) {
		x += incremental;
		return x % m;
	}
}