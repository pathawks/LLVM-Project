/**
 * CodeGen Redux
 */

#include <iostream>
#include <fstream>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

using namespace std;
using namespace llvm;

class Codegen {
	public:
	void header() {
		cout << ".global _main" << endl
		     << ".equ _main, main" << endl
		;
	}

	string instruction(string in, string op1="", string op2="") {
		string out = "\t"+in;
		if (op1.length()) {
			out +=  "\t" + op1;
		}
		if (op2.length()) {
			out += ",\t" + op2;
		}
		return out;
	}
};

Module& open() {
	SMDiagnostic error;
	LLVMContext context;
	return *(parseIRFile("HelloWorld.bc", error, context).get());
}

int main(int argc, char** argv) {
	SMDiagnostic error;
	LLVMContext context;
	Module &Mod = *(parseIRFile("HelloWorld.bc", error, context).get());
	cout << "Platform: " << Mod.getTargetTriple() << endl;
	cout << "Instructions: " << Mod.getInstructionCount() << endl;
	cout << "Id: " << Mod.getModuleIdentifier() << endl;
	cout << "Source File: " << Mod.getSourceFileName() << endl;
	for (const auto &g : Mod.globals()) {
		cout << "global: " << g.getName().str() << endl;
	}
//	Mod->dump();
//	Codegen c;
//	cout << c.instruction("retq") << endl;
//	cout << c.instruction("push", "$1") << endl;
//	cout << c.instruction("movq", "%rbp", "%rsp") << endl;
}
