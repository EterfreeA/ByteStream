﻿#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <algorithm>

#include "Core/Common.hpp"

ETERFREE_SPACE_BEGIN

// 获取错误描述
bool formatError(std::string& _buffer, std::uint64_t _error);
bool formatError(std::wstring& _buffer, std::uint64_t _error);

// 获取模块路径
bool getImagePath(std::string& _path);
bool getImagePath(std::wstring& _path);

// 暂停指定时间
void sleepFor(std::chrono::steady_clock::rep _duration);

//template <typename _Type>
//_Type reverse(_Type _value) noexcept
//{
//	constexpr auto SIZE = sizeof(_Type) - 1;
//
//	auto low = reinterpret_cast<char*>(&_value);
//	auto high = low + SIZE;
//
//	while (low < high)
//	{
//		auto byte = *low;
//		*low++ = *high;
//		*high-- = byte;
//	}
//	return _value;
//}

template <typename _Type>
_Type reverse(_Type _value) noexcept
{
	auto begin = reinterpret_cast<char*>(&_value);
	auto end = begin + sizeof _value;
	std::reverse(begin, end);
	return _value;
}

ETERFREE_SPACE_END
