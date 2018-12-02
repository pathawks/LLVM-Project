llvm-path = $(shell brew --prefix)/opt/llvm/bin
llvm-config = $(llvm-path)/llvm-config
CLANG = $(llvm-path)/clang
CXXFLAGS = -g -Wall -std=c++11 `$(llvm-config) --cppflags`
LDFLAGS = -g `$(llvm-config) --ldflags --system-libs --libs`

default: build/ codegen examples/Add.bc examples/HelloWorld.bc examples/Loop.bc examples/Sort.bc

build/codegen.o: src/codegen.cpp src/compile.hpp src/label.hpp

codegen: build/codegen.o build/compile.o build/op.o build/label.o build/stack.o

build:
	mkdir build

clean:
	rm -rf build/* examples/*.bc examples/*.ll codegen a.out

%.ll: %.c
	$(CLANG) -S -emit-llvm -O0 -o $@ $^

%.bc: %.c
	$(CLANG) -c -emit-llvm -O0 -o $@ $^

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%: build/%.o
	$(CXX) -v $(LDFLAGS) -o $@ $^
