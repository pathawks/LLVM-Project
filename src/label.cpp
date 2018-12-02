/**
 * Maps BasicBlock to label
 */

#include <sstream>
#include <string>
#include <map>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include "label.hpp"

std::string getLabel(const llvm::BasicBlock *block) {
	return getLabel(block, "BasicBlock_");
}

std::string getLabel(const llvm::BasicBlock &block) {
	return getLabel(&block);
}

std::string getLabel(const llvm::Value* block, std::string prefix) {
	static std::map<const llvm::Value*, std::string> blocks;
	static int num;
	std::string label;
	try {
		label = blocks.at(block);
	} catch (const std::out_of_range& oor) {
		std::stringstream s;
		s << prefix<< ++num;
		label = s.str();
		blocks.insert(std::pair<const llvm::Value*, std::string>(block, label));
	}
	return label;
}
