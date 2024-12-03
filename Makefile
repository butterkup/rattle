CUR_DIR=.
PREXT=ii
INC_DIR=$(CUR_DIR)/include
SRC_DIR=$(CUR_DIR)/src
LIB_DIR=$(CUR_DIR)/lib
LIBRARY=$(LIB_DIR)/librattle.so
PROGRAM=rattle

CXX=clang++
CXXFLAGS=-std=c++20 -fPIC -Wall -I$(INC_DIR)

ifeq ($(build),release)
	CXXFLAGS+=-O2
else
	CXXFLAGS+=-g
endif


SOURCES=$(shell find $(SRC_DIR) -name '*.cpp' -type f)
OBJECTS=$(subst .cpp,.o,$(SOURCES))
PREPROCESSED=$(subst .hpp,.$(PREXT),$(shell find $(INC_DIR) -name '*.hpp' -type f))
PREPROCESSED+=$(subst .cpp,.$(PREXT),$(SOURCES))
TO_CLEAN=$(OBJECTS) $(PROGRAM) $(PROGRAM).o $(LIBRARY) $(PREPROCESSED)

all: $(PROGRAM)

.PHONY+=process
process: $(PREPROCESSED)

$(PROGRAM): $(PROGRAM).o $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(DEFS) -o $@ $< -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -lrattle

$(LIBRARY): $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(CXX) $(CXXFLAGS) -shared -fPIC $(OBJECTS) -o $@

define preprocess_recipe
	@echo $@
	@$(CXX) $(CXXFLAGS) -E -o $@ $< 2>/dev/null || :
endef

%.$(PREXT): %.hpp
	$(preprocess_recipe)

%.$(PREXT): %.cpp
	$(preprocess_recipe)

.PHONY+=clean
clean:
	@echo CleaningUp
	@rm -rfv $(TO_CLEAN)
	@if [ -e "$(LIB_DIR)" ]; then rmdir --ignore-fail-on-non-empty "$(LIB_DIR)"; fi;

