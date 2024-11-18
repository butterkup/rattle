CUR_DIR=.
INCLUDE_DIR=$(CUR_DIR)/include
SRC_DIR=$(CUR_DIR)/src
LIB_DIR=$(CUR_DIR)/lib
LIBRARY=$(LIB_DIR)/librattle.so
PROGRAM=main

CXX=g++ -std=c++20 -I$(INCLUDE_DIR)
CXXFLAGS=-fPIC -Wall -O2

SOURCES=$(shell find $(SRC_DIR) -name '*.cpp' -type f)
OBJECTS=$(subst .cpp,.o,$(SOURCES))
TO_CLEAN=$(OBJECTS) $(PROGRAM) $(LIBRARY)

all: $(PROGRAM)

$(PROGRAM): main.cpp $(LIBRARY)
	$(CXX) -Wall -O2 -o $@ $< -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -lrattle

$(LIBRARY): $(OBJECTS)
	mkdir -p $(LIB_DIR)
	$(CXX) -shared -fPIC $^ -o $@

.PHONY+=clean
clean:
	rm -rfv $(TO_CLEAN)

