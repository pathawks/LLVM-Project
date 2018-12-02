/**
 * CodeGen Redux
 */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

#include "compile.hpp"
#include "op.hpp"

using namespace std;
using namespace llvm;

string header(Module *m) {
	stringstream s;
	s <<         "# Platform: " << m->getTargetTriple()
	  << "\n"    "# Instructions: " << m->getInstructionCount()
	  << "\n"    "# Id: " << m->getModuleIdentifier()
	  << "\n"    "# Source File: " << m->getSourceFileName()
	  << "\n"    ".global _main"
	  << "\n"    ".equ _main, main"
	;
	return s.str();
}

int main(int argc, char** argv) {
	SMDiagnostic error;
	LLVMContext context;
	std::unique_ptr<Module> mod;
	if (argc > 1 && strcmp(argv[1],"-")) {
		string inputFile = argv[1];
		mod = parseIRFile(inputFile, error, context, false);
		if (!mod) {
			cerr << "Could not open input file: " << inputFile << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		cerr << "No input file specified" "\n"
		        "Reading LLVM-IR from stdin"
		     << endl;
		mod = parseIRFile("-", error, context, false);
	}
	Module *m = mod.get();

	cout << header(m) << endl;

	for (Function &f: m->functions()) {
		if (!f.getInstructionCount()) {
			cout << "\n.equ " << f.getName().str() << ", _" << f.getName().str()
			     << "\t# External function" << endl;
			continue;
		}
		cout << "\n" << f.getName().str() << ":\t# Function"
		        "\n"    "\tpushq\t%rbp    \t# Save Old Base Pointer"
		        "\n"    "\tmovq\t%rsp,\t%rbp\t# Save Old Stack Pointer"
		     << endl;
		for (BasicBlock &block: f.getBasicBlockList()) {
			outs() << "Label_" << block.getValueID() << ":\n"
			          "# Does this block have a name? " << (block.hasName()?"Yes":"No")
			       << "\n# What is the block name?      " << block.getName();
			for (Instruction &instruction: block) {
				outs() << "\n\t#\t" << instruction << "\n\t" << compile(instruction);
			}
			outs().flush();
		}
	}

	for (const GlobalVariable &g : m->globals()) {
		cout << "\n" << g.getName().str() << ": " << op(&g) << endl;
	}
}
