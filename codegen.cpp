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

string op(User *v) {
	return "constant";
}

string op(Value *v) {
	stringstream s;
	switch(1) {
	case 0:
		s << ((Constant*)v)->getUniqueInteger().getLimitedValue();
	default:
		s << v->getType()->getPrimitiveSizeInBits()/8;
	}
	return s.str();
}

string compile(Instruction &i) {
	stringstream s;
	switch (i.getOpcode()) {
	case Instruction::Alloca:
		s << "subq\t$8, %rsp";
		break;
	case Instruction::Store:
		s << "movq\t$" << op(i.getOperand(0)) << ", " << op(i.getOperand(1));
		break;
	case Instruction::Load:
		s << "movq\t" << op(i.getOperand(0)) << ", %(rsp)";
		break;
	case Instruction::Call:
		s << "callq\t";
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

	cout <<         "# Platform: " << m->getTargetTriple()
	     << "\n" << "# Instructions: " << m->getInstructionCount()
	     << "\n" << "# Id: " << m->getModuleIdentifier()
	     << "\n" << "# Source File: " << m->getSourceFileName()
	     << "\n" << ".global _main\n.equ _main, main"
	     << endl;

	for (Function &f: m->functions()) {
		if (!f.getInstructionCount()) {
			cout << "\n.equ " << f.getName().str() << ", _" << f.getName().str() << endl;
			continue;
		}
		cout << "\n" << f.getName().str() << ":\t# Function"
		     << "\n" << "\tmovq\t%rsp, %rbp\t# Save Old Stack Pointer"
		     << endl;
		for (BasicBlock &block: f.getBasicBlockList()) {
			for (Instruction &instruction: block) {
				outs() << "\t" << compile(instruction) << "\t#\t" << instruction << "\n";
			}
		}
	}

	for (const auto &g : m->globals()) {
		cout << "\nglobal: " << g.getName().str() << endl;
	}
}
