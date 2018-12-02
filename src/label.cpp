/**
 * Maps BasicBlock to label
 */

#include <sstream>
#include <string>
#include <map>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

std::string getLabel(const llvm::BasicBlock *block) {
	static std::map<const llvm::BasicBlock*, std::string> blocks;
	static int num;
	if (blocks.count(block) == 0) {
		std::stringstream s;
		s << "BasicBlock_" << ++num;
		blocks.insert(std::pair<const llvm::BasicBlock*, std::string>(block, s.str()));
	}
	return blocks[block];
}

std::string getLabel(const llvm::BasicBlock &block) {
	return getLabel(&block);
}
