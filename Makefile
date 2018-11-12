llvm-path = $(shell brew --prefix)/opt/llvm/bin
llvm-config = $(llvm-path)/llvm-config
CLANG = $(llvm-path)/clang
CXXFLAGS = -g -Wall -std=c++11 `$(llvm-config) --cppflags`
LDFLAGS = -g `$(llvm-config) --ldflags --system-libs --libs`

default: codegen

codegen: codegen.o

clean:
	rm -rf *.o *.bc codegen

HelloWorld.bc: HelloWorld.c
	$(CLANG) -c -emit-llvm -O0 -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

%: %.o
	$(CXX) -v $(LDFLAGS) -o $@ $^
