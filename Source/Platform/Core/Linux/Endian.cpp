#include "Platform/Core/Endian.h"
#include "Platform/Core/Common.h"

#include <arpa/inet.h>

PLATFORM_SPACE_BEGIN

std::uint16_t hton(std::uint16_t _value)
{
	return htons(_value);
}

std::uint32_t hton(std::uint32_t _value)
{
	return htonl(_value);
}

std::uint64_t hton(std::uint64_t _value)
{
	return little() ? \
		reverse(_value) : _value;
}

std::uint32_t hton(float _value)
{
	auto value = *reinterpret_cast<std::uint32_t*>(&_value);
	return htonl(value);
}

std::uint64_t hton(double _value)
{
	if (little())
		_value = reverse(_value);
	return *reinterpret_cast<std::uint64_t*>(&_value);
}

//template <typename _Source, typename _Target>
//_Target ntoh(_Source _value)
//{
//	auto value = static_cast<_Target>(_value);
//	return little() ? reverse(value) : value;
//}

template <>
std::uint16_t ntoh(std::uint16_t _value)
{
	return ntohs(_value);
}

template <>
std::uint32_t ntoh(std::uint32_t _value)
{
	return ntohl(_value);
}

template <>
std::uint64_t ntoh(std::uint64_t _value)
{
	return little() ? \
		reverse(_value) : _value;
}

template <>
float ntoh(std::uint32_t _value)
{
	_value = ntohl(_value);
	return *reinterpret_cast<float*>(&_value);
}

template <>
double ntoh(std::uint64_t _value)
{
	if (little())
		_value = reverse(_value);
	return *reinterpret_cast<double*>(&_value);
}

PLATFORM_SPACE_END
