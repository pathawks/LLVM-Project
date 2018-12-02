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
	std::string label;
	try {
		label = blocks.at(block);
	} catch (const std::out_of_range& oor) {
		std::stringstream s;
		s << "BasicBlock_" << ++num;
		label = s.str();
		blocks.insert(std::pair<const llvm::BasicBlock*, std::string>(block, label));
	}
	return label;
}

std::string getLabel(const llvm::BasicBlock &block) {
	return getLabel(&block);
}
