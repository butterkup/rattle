CXX=g++ -std=c++20
CXXFLAGS=-fPIC -Wall -Iinclude

SOURCES=src/lexer/number.cpp src/lexer/string.cpp src/lexer/scanner.cpp src/lexer/state.cpp src/lexer/utility.cpp
OBJECTS=$(subst .cpp,.o,$(SOURCES))

main: main.cpp librattle.so
	$(CXX) -Wall -Iinclude -o $@ $< -L. -Wl,-rpath=. -lrattle

librattle.so: $(OBJECTS)
	$(CXX) -shared -fPIC $^ -o $@

$(OBJECTS): $(SOURCES)

.PHONY+=clean
clean:
	rm -rfv $(OBJECTS) librattle.so main

