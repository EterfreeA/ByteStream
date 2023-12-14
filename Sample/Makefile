IGNORE := .
ROOT := ..
SOURCE := $(ROOT)/Source
INCLUDE := $(ROOT)/Source
BINARY := $(ROOT)/Binary

CXXFLAGS := -std=c++20 -I$(INCLUDE)

TARGET := $(BINARY)/test

OBJECTS :=
OBJECTS += $(SOURCE)/Eterfree/Core/ByteStream.o
OBJECTS += $(SOURCE)/Eterfree/Platform/Core/Linux/Endian.o
OBJECTS += test.o

default: $(OBJECTS)
	${CXX} $^ -o $(TARGET)
%.o: %.cpp
	${CXX} $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGET)
