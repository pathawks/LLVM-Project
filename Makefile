default: codegen

llvm-path = $(shell brew --prefix)/opt/llvm/bin
llvm-config = $(llvm-path)/llvm-config
CXXFLAGS = -g -Wall -std=c++11 `$(llvm-config) --cppflags`
LDFLAGS = -g `$(llvm-config) --ldflags --system-libs --libs`

clean:
	rm -rf *.o codegen

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

codegen: codegen.o
	$(CXX) -v $(LDFLAGS) -o $@ $^
