/**
 * Maps BasicBlock to label
 */

#include <sstream>
#include <string>
#include <map>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

static std::map<const llvm::Value*, std::string> stackVars;
static int num;

std::string getStackPosition(const llvm::Instruction *alloca) {
	std::string label;
	try {
		label = stackVars.at(alloca);
	} catch (const std::out_of_range& oor) {
		std::stringstream s;
		s << ++num * -8 << "(%rbp)";
		label = s.str();
		stackVars.insert(std::pair<const llvm::Value*, std::string>{alloca, label});
	}
	return label;
}

void resetStack() {
	num = 0;
}
