#include "ByteStream.h"
#include "Eterfree/Platform/Core/Endian.h"

#include <utility>
#include <climits>
#include <cstring>

ETERFREE_SPACE_BEGIN

using namespace Platform;

auto ByteStream::getMaxSize(SizeType _maxSize, \
	bool _checksum) noexcept -> SizeType
{
	if (_maxSize <= 0) _maxSize = MAX_SIZE;

	if (_maxSize < SIZE) return 0;

	_maxSize -= SIZE;
	if (_checksum)
		_maxSize = _maxSize >= SIZE ? \
			_maxSize - SIZE : 0;
	return _maxSize;
}

auto ByteStream::calculateSum(const char* _data, \
	SizeType _size, bool _endian) -> StreamSize
{
	constexpr auto SIZE_BIT = SIZE * CHAR_BIT;

	constexpr auto ntohss = ntoh<StreamSize, StreamSize>;

	auto size = _size % SIZE;
	_size -= size;

	std::uint64_t sum = 0;
	auto address = reinterpret_cast<SizeType>(_data);
	if (address % ALIGNMENT == 0)
		for (decltype(_size) index = 0; \
			index < _size; index += SIZE)
		{
			auto data = _data + index;
			auto value = *reinterpret_cast<const StreamSize*>(data);
			if (_endian) value = ntohss(value);
			sum += value;
		}
	else
		for (decltype(_size) index = 0; \
			index < _size; index += SIZE)
		{
			StreamSize value = 0;
			std::memcpy(&value, _data + index, SIZE);
			if (_endian) value = ntohss(value);
			sum += value;
		}

	if (size > 0)
	{
		StreamSize value = 0;
		std::memcpy(&value, _data + _size, size);
		if (_endian) value = ntohss(value);
		sum += value;
	}

	while (sum > MAX_SIZE)
		sum = (sum & MAX_SIZE) + (sum >> SIZE_BIT);
	return static_cast<StreamSize>(sum);
}

auto ByteStream::convertSum(StreamSize _sum, \
	bool _endian) -> StreamSize
{
	_sum = ~_sum;
	if (_endian) _sum = hton(_sum);
	return _sum;
}

bool ByteStream::checkSum(const char* _data, \
	StreamSize _sum, bool _endian)
{
	decltype(_sum) sum = 0;
	auto address = reinterpret_cast<SizeType>(_data);
	if (address % ALIGNMENT == 0)
		sum = *reinterpret_cast<const StreamSize*>(_data);
	else
		std::memcpy(&sum, _data, SIZE);

	if (_endian)
		sum = ntoh<StreamSize, StreamSize>(sum);
	return _sum + sum == MAX_SIZE;
}

void ByteStream::clearFlag() noexcept
{
	FlagType flag;
	resetFlag(flag);
	_flag.store(flag, \
		std::memory_order::relaxed);
}

auto OutputByteStream::getSize(SizeType _offset) const \
-> StreamSize
{
	StreamSize size = 0;
	auto data = _buffer.data() + _offset;
	auto address = reinterpret_cast<SizeType>(data);
	if (address % ALIGNMENT == 0)
		size = *reinterpret_cast<const StreamSize*>(data);
	else
		std::memcpy(&size, data, SIZE);

	if (existFlag(FLAG_TYPE_ENDIAN))
		size = ntoh<StreamSize, StreamSize>(size);
	return size;
}

void OutputByteStream::limit(SizeType _maxSize, \
	SizeType _capacity) noexcept
{
	storeMaxSize(_maxSize);

	this->_capacity.store(_capacity, \
		std::memory_order::relaxed);
}

bool OutputByteStream::idle() const noexcept
{
	auto capacity = this->capacity();
	return capacity <= 0 \
		or _queue.size() < capacity;
}

const char* OutputByteStream::data(SizeType& _size)
{
	auto flag = loadFlag();
	bool endian = existFlag(flag, FLAG_TYPE_ENDIAN);
	bool checksum = existFlag(flag, FLAG_TYPE_CHECKSUM);

	auto maxSize = loadMaxSize();
	maxSize = getMaxSize(maxSize, checksum);
	while (_buffer.size() - _offset < _size \
		and not _queue.empty())
	{
		const auto& packet = _queue.front();
		if (_buffer.size() > maxSize \
			or packet.size() > maxSize - _buffer.size())
			break;

		auto size = static_cast<StreamSize>(packet.size());
		if (endian) size = hton(size);

		auto data = reinterpret_cast<const char*>(&size);
		_buffer.append(data, SIZE);

		// 生成特定累加和
		if (checksum)
		{
			auto sum = calculateSum(packet.data(), \
				packet.size(), endian);
			sum = convertSum(sum, endian);

			data = reinterpret_cast<const char*>(&sum);
			_buffer.append(data, SIZE);
		}

		_buffer.append(packet);
		_queue.pop_front();
	}

	_size = _buffer.size() - _offset;
	return _buffer.data() + _offset;
}

bool OutputByteStream::put(const char* _data, \
	SizeType _size)
{
	if (_data == nullptr and _size != 0)
		return false;

	bool checksum = existFlag(FLAG_TYPE_CHECKSUM);
	auto maxSize = loadMaxSize();
	maxSize = getMaxSize(maxSize, checksum);
	if (_size > maxSize) return false;

	if (not idle()) return false;

	_queue.emplace_back(_data, _size);
	return true;
}

void OutputByteStream::take(SizeType _size)
{
	if (_size >= _buffer.size() - _offset)
	{
		_offset = 0;
		_buffer.clear();
		return;
	}

	_offset += _size;

	auto extraSize = SIZE;
	if (existFlag(FLAG_TYPE_CHECKSUM))
		extraSize += SIZE;

	decltype(_offset) offset = 0;
	decltype(offset) totalSize = 0;
	do
	{
		offset += totalSize;
		if (_offset - offset < extraSize)
			break;

		auto size = getSize(offset);
		totalSize = extraSize + size;
	} while (_offset - offset >= totalSize);

	if (offset > 0)
	{
		_buffer.erase(0, offset);
		_offset -= offset;
	}
}

void OutputByteStream::clear() noexcept
{
	_queue.clear();

	_offset = 0;
	_buffer.clear();
}

bool InputByteStream::getSize()
{
	if (_buffer.size() - _offset < SIZE)
		return false;

	StreamSize size = 0;
	auto data = _buffer.data() + _offset;
	auto address = reinterpret_cast<SizeType>(data);
	if (address % ALIGNMENT == 0)
		size = *reinterpret_cast<const StreamSize*>(data);
	else
		std::memcpy(&size, data, SIZE);

	if (existFlag(FLAG_TYPE_ENDIAN))
		size = ntoh<StreamSize, StreamSize>(size);

	_size = size;
	_offset += static_cast<decltype(_offset)>(SIZE);
	return true;
}

bool InputByteStream::getPacket()
{
	bool result = true;
	decltype(_size) size = 0;

	// 检验特定累加和
	auto flag = loadFlag();
	if (existFlag(flag, FLAG_TYPE_CHECKSUM))
	{
		bool endian = existFlag(flag, \
			FLAG_TYPE_ENDIAN);
		auto data = _buffer.data() + _offset;

		size = static_cast<decltype(size)>(SIZE);
		auto sum = calculateSum(data + size, \
			_size, endian);
		result = checkSum(data, sum, endian);
	}

	if (result)
		_queue.emplace_back(_buffer, \
			_offset + size, _size);
	return result;
}

bool InputByteStream::flushBuffer()
{
	bool result = true;
	decltype(_offset) offset = 0;

	StreamSize extraSize = 0;
	if (existFlag(FLAG_TYPE_CHECKSUM))
		extraSize = static_cast<StreamSize>(SIZE);

	do
	{
		if (_size <= 0 \
			and not getSize())
			break;

		auto size = _buffer.size() - _offset;
		if (size >= _size \
			and size - _size >= extraSize \
			and idle())
		{
			result = getPacket();

			_offset += _size + extraSize;
			offset = _offset;
			_size = 0;
		}
	} while (_size <= 0 and result);

	if (offset > 0)
	{
		_buffer.erase(0, offset);
		_offset -= offset;
	}
	return result;
}

void InputByteStream::limit(SizeType _maxSize, \
	SizeType _capacity) noexcept
{
	storeMaxSize(_maxSize);

	this->_capacity.store(_capacity, \
		std::memory_order::relaxed);
}

bool InputByteStream::idle() const noexcept
{
	auto capacity = this->capacity();
	return capacity <= 0 \
		or _queue.size() < capacity;
}

bool InputByteStream::put(const char* _data, \
	SizeType _size, SizeType& _offset)
{
	auto maxSize = loadMaxSize();
	if (maxSize <= 0) maxSize = MAX_SIZE;

	auto size = _buffer.size();
	if (size > maxSize) return false;

	size = maxSize - size;
	if ((_size -= _offset) > size)
		_size = size;

	_buffer.append(_data + _offset, _size);
	_offset += _size;
	return flushBuffer();
}

bool InputByteStream::put(const char* _data, \
	SizeType _size)
{
	decltype(_size) offset = 0;
	while (offset < _size)
		if (not put(_data, _size, offset))
		{
			reset();
			return false;
		}
	return true;
}

bool InputByteStream::take(Buffer& _packet) noexcept
{
	if (_queue.empty()) return false;

	_packet = std::move(_queue.front());
	_queue.pop_front();
	return true;
}

ETERFREE_SPACE_END
