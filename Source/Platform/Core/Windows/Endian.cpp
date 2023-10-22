#include "Platform/Core/Endian.h"
#include "Platform/Core/Common.h"

#include <WinSock2.h>
#pragma comment(lib, "WS2_32.Lib")

PLATFORM_SPACE_BEGIN

std::uint16_t hton(std::uint16_t _value)
{
	static_assert(sizeof _value == sizeof(u_short), \
		"The size of _value is not equal to the size of u_short.");
	return htons(_value);
}

std::uint32_t hton(std::uint32_t _value)
{
	static_assert(sizeof _value == sizeof(u_long), \
		"The size of _value is not equal to the size of u_long.");
	return htonl(_value);
}

std::uint64_t hton(std::uint64_t _value)
{
	return htonll(_value);
}

std::uint32_t hton(float _value)
{
	return htonf(_value);
}

std::uint64_t hton(double _value)
{
	return htond(_value);
}

template <>
std::uint16_t ntoh(std::uint16_t _value)
{
	static_assert(sizeof _value == sizeof(u_short), \
		"The size of _value is not equal to the size of u_short.");
	return ntohs(_value);
}

template <>
std::uint32_t ntoh(std::uint32_t _value)
{
	static_assert(sizeof _value == sizeof(u_long), \
		"The size of _value is not equal to the size of u_long.");
	return ntohl(_value);
}

template <>
std::uint64_t ntoh(std::uint64_t _value)
{
	return ntohll(_value);
}

template <>
float ntoh(std::uint32_t _value)
{
	return ntohf(_value);
}

template <>
double ntoh(std::uint64_t _value)
{
	return ntohd(_value);
}

PLATFORM_SPACE_END
