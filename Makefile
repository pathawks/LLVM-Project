default: codegen

CXXFLAGS = -Wall -std=c++11

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

codegen: codegen.o
	$(CXX) -o $@ $^
