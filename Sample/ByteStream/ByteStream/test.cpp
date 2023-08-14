#include "Core/ByteStream.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <random>
#include <iostream>

USING_ETERFREE_SPACE

static bool check()
{
	using ValueType = std::uint32_t;
	using ByteType = std::uint8_t;

	using SizeType = ByteStream::SizeType;
	using Buffer = ByteStream::Buffer;

	constexpr auto ALIGNMENT = alignof(ValueType);
	constexpr auto SIZE = sizeof(ValueType);

	constexpr SizeType NUMBER = 10;
	constexpr SizeType EXTRA_BYTES = 1;

	constexpr auto LENGTH = SIZE * NUMBER + EXTRA_BYTES;

	Buffer buffer(ALIGNMENT + LENGTH + SIZE, '\0');
	auto pointer = buffer.data();
	auto address = reinterpret_cast<SizeType>(pointer);
	auto offset = address % ALIGNMENT;
	if (offset <= 0) offset = ALIGNMENT;

	*pointer = static_cast<Buffer::value_type>(offset);
	pointer += offset;

	std::random_device device;
	std::uniform_int_distribution<ValueType> random;
	for (SizeType index = 0; index < NUMBER; ++index)
	{
		auto value = reinterpret_cast<ValueType*>(pointer);
		*value = random(device);
		pointer += sizeof *value;
	}

	for (SizeType index = 0; index < EXTRA_BYTES; ++index)
	{
		auto value = reinterpret_cast<ByteType*>(pointer);
		*value = random(device) % UINT8_MAX;
		++pointer;
	}

	pointer = buffer.data();
	pointer += *pointer;
	auto sum = ByteStream::calculateSum(pointer, LENGTH, true);
	sum = ByteStream::convertSum(sum, true);

	pointer += LENGTH;
	std::memcpy(pointer, &sum, sizeof sum);

	pointer = buffer.data();
	pointer += *pointer;
	sum = ByteStream::calculateSum(pointer, LENGTH, true);

	pointer += LENGTH;
	return ByteStream::checkSum(pointer, sum, true);
}

static void move(OutputByteStream& _output, InputByteStream& _input)
{
	using SizeType = ByteStream::SizeType;

	constexpr SizeType SIZE = 16;

	while (not _output.empty())
	{
		auto size = SIZE;
		auto data = _output.data(size);

		SizeType offset = 0;
		while (offset < size)
			// 数据错误，断开连接
			if (not _input.put(data, size, offset)) break;

		_output.take(size);
	}
}

static void take(InputByteStream& _input, \
	ByteStream::QueueType& _queue)
{
	if (_input.take(_queue))
	{
		bool result = false;
		ByteStream::Buffer buffer;
		do
		{
			_input.flush();
			result = _input.take(buffer);
			if (result)
				_queue.push_back(buffer);
		} while (result);
	}
}

static const char* SENTENCE = "一切皆有可能";
static const char* PARAGRAPH[] = {
	"黯然销魂掌",
	"相思无用，唯别而已。别期若有定，千般煎熬又何如？莫道黯然销魂，何处柳暗花明。",
	"一掌黯然鬼神皆寂寥"
};

int main()
{
	using SizeType = ByteStream::SizeType;

	using std::cout, std::boolalpha, std::endl;

	constexpr SizeType CAPACITY = 2;
	//constexpr SizeType SIZE = 128;
	constexpr SizeType SIZE = 32;

	cout << "checksum " << boolalpha << check() << endl;

	ByteStream::FlagType flag;
	ByteStream::resetFlag(flag);
	ByteStream::setFlag(flag, ByteStream::FLAG_TYPE_ENDIAN);
	ByteStream::setFlag(flag, ByteStream::FLAG_TYPE_CHECKSUM);

	OutputByteStream output;
	output.limit(CAPACITY, SIZE);
	output.replaceFlag(flag);

	InputByteStream input;
	input.limit(CAPACITY, SIZE);
	input.replaceFlag(flag);

	cout << "\noutput\n";
	cout << boolalpha << output.put(SENTENCE) \
		<< ' ' << SENTENCE << endl;

	move(output, input);

	for (auto sentence : PARAGRAPH)
		cout << boolalpha << output.put(sentence) \
		<< ' ' << sentence << endl;

	move(output, input);

	ByteStream::QueueType queue;
	take(input, queue);

	cout << "\ninput\n";
	for (const auto& packet : queue)
		cout << packet << endl;
	return EXIT_SUCCESS;
}
