# LLVM Project

## To build

```sh
make
```

## To run

```sh
./codegen examples/Sort.bc > Sort.s
gcc Sort.s -o Sort
./Sort
```
