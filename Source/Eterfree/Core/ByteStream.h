#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <deque>
#include <atomic>

#include "BitSet.hpp"
#include "Common.hpp"

ETERFREE_SPACE_BEGIN

class ByteStream
{
public:
	enum FLAG_TYPE : std::uint32_t
	{
		FLAG_TYPE_ENDIAN,
		FLAG_TYPE_CHECKSUM,
	};

protected:
	using StreamSize = std::uint32_t;

public:
	using FlagType = std::uint32_t;
	using SizeType = std::size_t;

	using Buffer = std::string;
	using QueueType = std::deque<Buffer>;

protected:
	static constexpr auto ALIGNMENT = alignof(StreamSize);
	static constexpr auto SIZE = sizeof(StreamSize);

public:
	static constexpr auto MAX_SIZE = UINT32_MAX;

protected:
	std::atomic<FlagType> _flag;
	std::atomic<SizeType> _maxSize;

public:
	static bool existFlag(FlagType _flag, FLAG_TYPE _type) noexcept
	{
		return existBit(_flag, static_cast<FlagType>(_type));
	}

	static void resetFlag(FlagType& _flag) noexcept
	{
		resetBit(_flag);
	}

	static void setFlag(FlagType& _flag, FLAG_TYPE _type, \
		bool _enabled = true) noexcept
	{
		setBit(_flag, static_cast<FlagType>(_type), _enabled);
	}

	static SizeType getMaxSize(SizeType _maxSize, \
		bool _checksum) noexcept;

	static StreamSize calculateSum(const char* _data, \
		SizeType _size, bool _endian);

	static StreamSize convertSum(StreamSize _sum, bool _endian);

	static bool checkSum(const char* _data, \
		StreamSize _sum, bool _endian);

protected:
	auto loadMaxSize() const noexcept
	{
		return _maxSize.load(std::memory_order::relaxed);
	}

	void storeMaxSize(SizeType _maxSize) noexcept
	{
		this->_maxSize.store(_maxSize, \
			std::memory_order::relaxed);
	}

public:
	ByteStream(SizeType _maxSize = 0) noexcept : \
		_flag(0), _maxSize(_maxSize) {}

	virtual ~ByteStream() noexcept {}

	auto loadFlag() const noexcept
	{
		return _flag.load(std::memory_order::relaxed);
	}

	bool existFlag(FLAG_TYPE _type) const noexcept
	{
		return existFlag(loadFlag(), _type);
	}

	auto insertFlag(FlagType _flag) noexcept
	{
		return this->_flag.fetch_or(_flag, \
			std::memory_order::relaxed);
	}

	auto removeFlag(FlagType _flag) noexcept
	{
		return this->_flag.fetch_and(~_flag, \
			std::memory_order::relaxed);
	}

	auto replaceFlag(FlagType _flag) noexcept
	{
		return this->_flag.exchange(_flag, \
			std::memory_order::relaxed);
	}

	void clearFlag() noexcept;
};

class OutputByteStream : public ByteStream
{
	std::atomic<SizeType> _capacity;
	QueueType _queue;

	SizeType _offset;
	Buffer _buffer;

private:
	StreamSize getSize(SizeType _offset) const;

public:
	OutputByteStream(SizeType _maxSize = 0, SizeType _capacity = 0) : \
		ByteStream(_maxSize), _capacity(_capacity), _offset(0) {}

	auto capacity() const noexcept
	{
		return _capacity.load(std::memory_order::relaxed);
	}

	void limit(SizeType _maxSize, SizeType _capacity) noexcept;

	bool empty() const noexcept
	{
		return _queue.empty() \
			and _buffer.empty();
	}

	bool idle() const noexcept;

	const char* data(SizeType& _size);

	bool put(const char* _data, SizeType _size);

	bool put(const Buffer& _buffer)
	{
		return put(_buffer.data(), _buffer.size());
	}

	void take(SizeType _size);

	void reset() noexcept
	{
		_offset = 0;
	}

	void clear() noexcept;
};

class InputByteStream : public ByteStream
{
	std::atomic<SizeType> _capacity;
	QueueType _queue;

	StreamSize _size, _offset;
	Buffer _buffer;

private:
	bool getSize();

	bool getPacket();

	bool flushBuffer();

public:
	InputByteStream(SizeType _maxSize = 0, SizeType _capacity = 0) : \
		ByteStream(_maxSize), _capacity(_capacity), _size(0), _offset(0) {}

	auto capacity() const noexcept
	{
		return _capacity.load(std::memory_order::relaxed);
	}

	void limit(SizeType _maxSize, SizeType _capacity) noexcept;

	bool empty() const noexcept
	{
		return _queue.empty();
	}

	// 先调用idle，再进行receive，最后调用put
	bool idle() const noexcept;

	bool flush()
	{
		return flushBuffer();
	}

	bool put(const char* _data, SizeType _size, SizeType& _offset);

	bool put(const Buffer& _buffer, SizeType& _offset)
	{
		return put(_buffer.data(), _buffer.size(), _offset);
	}

	bool put(const char* _data, SizeType _size);

	bool put(const Buffer& _buffer)
	{
		return put(_buffer.data(), _buffer.size());
	}

	bool take(Buffer& _packet) noexcept;

	bool take(QueueType& _queue) noexcept
	{
		this->_queue.swap(_queue);
		return not _queue.empty();
	}

	void reset() noexcept
	{
		_size = _offset = 0;
		_buffer.clear();
	}

	void clear() noexcept
	{
		_queue.clear();
		reset();
	}
};

ETERFREE_SPACE_END
