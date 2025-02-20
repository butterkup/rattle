CUR_DIR=.
INC_DIR=$(CUR_DIR)/include
SRC_DIR=$(CUR_DIR)/src
LIB_DIR=$(CUR_DIR)/lib
LIBRARY=$(LIB_DIR)/librattle.so
DEPMK=$(CUR_DIR)/ignore.deps.mk
PROGRAM=$(CUR_DIR)/rattle

CXX=clang++
CXXFLAGS=-std=c++20 -pedantic -fsanitize=undefined -fPIC -Wall -I$(INC_DIR)

ifeq ($(build),release)
	CXXFLAGS+=-O2 -DNDEBUG
else
	CXXFLAGS+=-Og -g
endif

SOURCES=$(shell find $(SRC_DIR) -name '*.cpp' -type f) $(PROGRAM).cpp
HEADERS=$(shell find $(INC_DIR) -name '*.hpp' -type f)
DEPS=$(subst .cpp,.cpp.d,$(SOURCES))
OBJECTS=$(subst .cpp,.o,$(SOURCES))
PREPROCESS=$(subst .hpp,.hpp.ii,$(HEADERS))
PREPROCESS+=$(subst .cpp,.cpp.ii,$(SOURCES))
STAT_TARGETS=$(HEADERS) $(SOURCES)

all: $(PROGRAM)

include $(DEPMK)

stat:
	@stats=("File Lines Words Bytes"); \
	while read lines words chars file; do \
	  stats+=("$$file $$lines $$words $$chars"); \
	done < <(wc --total=always $(STAT_TARGETS)); \
	IFS=$$'\n'; column -t <<<$${stats[*]}

.PHONY+=process
preprocess: $(PREPROCESS)

$(DEPMK): $(DEPS)
	@for file in $(DEPS); do \
		echo >>$@ "include $$file"; \
	done

.PHONY+=deps
deps: $(DEPMK)
	@echo Creating DependencyMK: '$(DEPMK)'

$(PROGRAM): $(PROGRAM).o $(LIBRARY)
	$(CXX) $(CXXFLAGS) $(DEFS) -o $@ $< -L$(LIB_DIR) -Wl,-rpath=$(LIB_DIR) -lrattle

$(LIBRARY): $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	@$(CXX) $(CXXFLAGS) -shared -fPIC $(OBJECTS) -o $@
	@echo Creating library: $@

define preprocess_recipe
	$(CXX) $(CXXFLAGS) -E -o $@ $<
endef

%.hpp.ii: %.hpp
	$(preprocess_recipe)

%.cpp.ii: %.cpp
	$(preprocess_recipe)

%.cpp.d: %.cpp
	@echo -n "$$(dirname '$<')/" >$@
	$(CXX) $(CXXFLAGS) -M $< >>$@

.PHONY+=clean_objects
clean_objects:
	@echo Removing ObjectFiles
	@rm -rfv $(OBJECTS)

.PHONY+=clean_preprocess
clean_preprocess:
	@echo Removing PreprocessorFiles
	@rm -rfv $(PREPROCESS)

.PHONY+=clean_$(PROGRAM)
clean_program:
	@echo Removing Exec\&Lib
	@rm -rfv $(PROGRAM) $(PROGRAM).o $(LIBRARY)
	@if [ -e "$(LIB_DIR)" ]; then rmdir --ignore-fail-on-non-empty "$(LIB_DIR)"; fi;

.PHONY+=clean_deps
clean_deps:
	@echo Removing DependencyMakefiles
	@rm -rfv $(DEPS) $(DEPMK)

.PHONY+=clean
clean: clean_objects clean_program clean_preprocess

.PHONY+=clean_all
clean_all: clean clean_deps

