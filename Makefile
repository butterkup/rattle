CUR_DIR=.
INC_DIR=$(CUR_DIR)/include
SRC_DIR=$(CUR_DIR)/src
LIB_DIR=$(CUR_DIR)/lib
LIBRARY=$(LIB_DIR)/librattle.so
PROGRAM=main

CXX=clang++
CXXFLAGS=-std=c++20 -fPIC -Wall -I$(INC_DIR)

ifeq ($(build),release)
	CXXFLAGS+=-O2
else
	CXXFLAGS+=-g
endif


SOURCES=$(shell find $(SRC_DIR) -name '*.cpp' -type f)
OBJECTS=$(subst .cpp,.o,$(SOURCES))
TO_CLEAN=$(OBJECTS) $(PROGRAM) $(PROGRAM).o $(LIBRARY)

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).o $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(DEFS) -o $@ $< -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -lrattle

$(LIBRARY): $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared -fPIC $(OBJECTS) -o $@

.PHONY+=clean
clean:
	rm -rfv $(TO_CLEAN)
	@[ -e $(LIB_DIR) ] && rmdir --ignore-fail-on-non-empty $(LIB_DIR)

