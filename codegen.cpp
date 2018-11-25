/**
 * CodeGen Redux
 */

#include <fstream>
#include <iostream>
#include <sstream>

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

string op(User *v) {
	return "constant";
}

string op(Value *v) {
	stringstream s;
	switch(1) {
	case 0:
		s << ((APInt*)v)->getLimitedValue();
	default:
		s << v->getType()->getPrimitiveSizeInBits()/8;
	}
	return s.str();
}

string compile(Instruction &i) {
	stringstream s;
	switch (i.getOpcode()) {
	case Instruction::Alloca:
		s << "subq\t$" << op(i.getOperand(0)) << ", %rsp";
		break;
	case Instruction::Store:
		s << "movq\t$" << op(i.getOperand(0)) << ", " << op(i.getOperand(1));
		break;
	case Instruction::Load:
	case Instruction::Call:
		s << "not yet implemented";
		break;
	case Instruction::Ret:
		s << "retq";
		break;
	default:
		s << "unknown instruction";
	}
	s << '\n';
	return s.str();
}

int main(int argc, char** argv) {
	SMDiagnostic error;
	LLVMContext context;
	std::unique_ptr<Module> mod = parseIRFile("Add.bc", error, context, false);
	Module *m = mod.get();

	cout << "Platform: " << m->getTargetTriple() << endl;
	cout << "Instructions: " << m->getInstructionCount() << endl;
	cout << "Id: " << m->getModuleIdentifier() << endl;
	cout << "Source File: " << m->getSourceFileName() << endl;

	for (Function &f: m->functions()) {
		if (!f.getInstructionCount()) continue;
		cout << f.getName().str() << ":" << endl;
		for (BasicBlock &block: f.getBasicBlockList()) {
			for (Instruction &instruction: block) {
				outs() << "\t" << compile(instruction) << "\t#\t" << instruction << "\n";
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
