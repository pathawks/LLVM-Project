/**
 * Maps BasicBlock to label
 */

#include <sstream>
#include <string>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

std::string getLabel(const llvm::BasicBlock *block);
std::string getLabel(const llvm::BasicBlock &block);
