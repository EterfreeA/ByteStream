#define STREAM 1
#define BIT_SET 2

#define TEST STREAM

#if TEST == STREAM
#include "ByteStream/test.cpp"

#elif TEST == BIT_SET
#include "BitSet/test.cpp"
#endif
