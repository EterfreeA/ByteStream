#pragma once

#include <cstdint>

#include "Common.h"

PLATFORM_SPACE_BEGIN

// 小端模式
inline bool little() noexcept
{
	constexpr auto INTEGER = static_cast<std::uint16_t>(1);
	return *reinterpret_cast<const char*>(&INTEGER) != '\0';
}

inline std::uint8_t hton(std::uint8_t _value) noexcept
{
	return _value;
}

std::uint16_t hton(std::uint16_t _value);

std::uint32_t hton(std::uint32_t _value);

std::uint64_t hton(std::uint64_t _value);

inline std::uint8_t hton(std::int8_t _value) noexcept
{
	return static_cast<std::uint8_t>(_value);
}

inline std::uint16_t hton(std::int16_t _value)
{
	return hton(static_cast<std::uint16_t>(_value));
}

inline std::uint32_t hton(std::int32_t _value)
{
	return hton(static_cast<std::uint32_t>(_value));
}

inline std::uint64_t hton(std::int64_t _value)
{
	return hton(static_cast<std::uint64_t>(_value));
}

inline std::uint8_t hton(bool _value) noexcept
{
	return static_cast<std::uint8_t>(_value);
}

std::uint32_t hton(float _value);

std::uint64_t hton(double _value);

template <typename _Source, typename _Target>
_Target ntoh(_Source _value);

template <>
inline std::uint8_t ntoh(std::uint8_t _value)
{
	return _value;
}

template <>
std::uint16_t ntoh(std::uint16_t _value);

template <>
std::uint32_t ntoh(std::uint32_t _value);

template <>
std::uint64_t ntoh(std::uint64_t _value);

template <>
inline std::int8_t ntoh(std::uint8_t _value)
{
	return static_cast<std::int8_t>(_value);
}

template <>
inline std::int16_t ntoh(std::uint16_t _value)
{
	auto value = ntoh<std::uint16_t, std::uint16_t>(_value);
	return static_cast<std::int16_t>(value);
}

template <>
inline std::int32_t ntoh(std::uint32_t _value)
{
	auto value = ntoh<std::uint32_t, std::uint32_t>(_value);
	return static_cast<std::int32_t>(value);
}

template <>
inline std::int64_t ntoh(std::uint64_t _value)
{
	auto value = ntoh<std::uint64_t, std::uint64_t>(_value);
	return static_cast<std::int64_t>(value);
}

template <>
inline bool ntoh(std::uint8_t _value)
{
	return static_cast<bool>(_value);
}

template <>
float ntoh(std::uint32_t _value);

template <>
double ntoh(std::uint64_t _value);

PLATFORM_SPACE_END
