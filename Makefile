CUR_DIR=.
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
HEADERS=$(shell find $(INC_DIR) -name '*.hpp' -type f)
OBJECTS=$(subst .cpp,.o,$(SOURCES))
PREPROCESSED=$(subst .hpp,.ii,$(HEADERS))
PREPROCESSED+=$(subst .cpp,.ii,$(SOURCES))
TO_CLEAN=$(OBJECTS) $(PROGRAM) $(PROGRAM).o $(LIBRARY) $(PREPROCESSED)
STAT_TARGETS=$(HEADERS) $(SOURCES)

all: $(PROGRAM)

stat:
	@stats=("File Lines Words Bytes"); \
	while read lines words chars file; do \
	  stats+=("$$file $$lines $$words $$chars"); \
	done < <(wc --total=always $(STAT_TARGETS)); \
	IFS=$$'\n'; column -t <<<$${stats[*]}

.PHONY+=process
process: $(PREPROCESSED)

$(PROGRAM): $(PROGRAM).o $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(DEFS) -o $@ $< -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -lrattle

$(LIBRARY): $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	@$(CXX) $(CXXFLAGS) -shared -fPIC $(OBJECTS) -o $@
	@echo Creating library: $@

define preprocess_recipe
	$(CXX) $(CXXFLAGS) -DRATTLE_SOURCE_ONLY -E -o $@ $<
endef

%.ii: %.hpp
	$(preprocess_recipe)

%.ii: %.cpp
	$(preprocess_recipe)

.PHONY+=clean
clean:
	@echo CleaningUp
	@rm -rfv $(TO_CLEAN)
	@if [ -e "$(LIB_DIR)" ]; then rmdir --ignore-fail-on-non-empty "$(LIB_DIR)"; fi;

