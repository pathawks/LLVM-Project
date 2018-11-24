/**
 * CodeGen Redux
 */

#include <iostream>
#include <fstream>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

//#include <llvm/ADT/STLExtras.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>

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
	InitializeNativeTarget();
	LLVMInitializeNativeAsmPrinter();
	LLVMInitializeNativeAsmParser();

	SMDiagnostic error;
	LLVMContext context;
	std::unique_ptr<Module> mod = parseIRFile("HelloWorld.bc", error, context, false);
	Module *m = mod.get();

	cout << "Platform: " << m->getTargetTriple() << endl;
	cout << "Instructions: " << m->getInstructionCount() << endl;
	cout << "Id: " << m->getModuleIdentifier() << endl;
	cout << "Source File: " << m->getSourceFileName() << endl;

	for (Function &f: m->functions()) {
		for (BasicBlock &block: f.getBasicBlockList()) {
			for (Instruction &instruction: block) {
				outs() << instruction << "\n";
			}
		}
	}

	for (const auto &g : m->globals()) {
		cout << "global: " << g.getName().str() << endl;
	}
//	Codegen c;
//	cout << c.instruction("retq") << endl;
//	cout << c.instruction("push", "$1") << endl;
//	cout << c.instruction("movq", "%rbp", "%rsp") << endl;
}
