#define BIT_SET 1
#define STREAM 2

#define TEST STREAM

#if TEST == BIT_SET
#include "BitSet/test.cpp"

#elif TEST == STREAM
#include "ByteStream/test.cpp"
#endif
