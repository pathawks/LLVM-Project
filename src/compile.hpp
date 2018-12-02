/**
 * Compile LLVM Instruction to x86 Assembly
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

std::string compile(llvm::Instruction &i);
