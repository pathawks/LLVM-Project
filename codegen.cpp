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

string arg(unsigned a) {
	switch(a) {
	case 0:
		return "%rdi";
	case 1:
		return "%rsi";
	case 2:
		return "%rdx";
	case 3:
		return "%rcx";
	case 4:
		return "%r8";
	case 5:
		return "%r9";
	default:
		stringstream s;
		s << (a-4)*-8 << "(%rbp)";
		return s.str();
	}
}

string op(Value *v) {
	stringstream s;
	if (ConstantInt* c = dyn_cast<ConstantInt>(v)) {
		s << '$' << c->getSExtValue();
	} else if (ConstantData* m = dyn_cast<ConstantData>(v)) {
		s << "ConstantData";
	} else if (Constant* m = dyn_cast<Constant>(v)) {
		s << "Constant";
	} else if (AllocaInst* a = dyn_cast<AllocaInst>(v)) {
		s << "(%rsp)" << op(a->getOperand(0));
	} else if (Instruction* m = dyn_cast<Instruction>(v)) {
		s << "Inst";
	} else if (Argument* a = dyn_cast<Argument>(v)) {
		s << arg(a->getArgNo());
	} else if (User* m = dyn_cast<User>(v)) {
		s << "User";
	} else {
		s << "??????????";
	}
	return s.str();
}

string compile(Instruction &i) {
	stringstream s;
	switch (i.getOpcode()) {
	case Instruction::Alloca:
		s << "subq\t$8,\t%rsp";
		break;
	case Instruction::Store:
		s << "movq\t" << op(i.getOperand(0)) << ",\t%r11";
		s << "\n\tmovq\t" "%r11" ",\t" << op(i.getOperand(1));
		break;
	case Instruction::Load:
		s << "movq\t" << op(i.getOperand(0)) << ",\t?r11";
		break;
	case Instruction::Call:
		s << "callq\t";
		break;
	case Instruction::Ret:
		s <<   "movq\t%rbp,\t%rsp\t# Restore Old Stack Pointer"
		     "\n\tpopq\t%rbp    \t# Restore Old Base Pointer"
		     "\n\tmovq\t" << op(i.getOperand(0)) << ",\t%rax\t# Set return value"
		     "\n\tretq            \t# Return from function"
		;
		break;
	case Instruction::Add:
		s << "addq\t" << op(i.getOperand(0)) << ",\t" << op(i.getOperand(1));
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
		        "\n"    "\tpushq\t%rbp    \t# Save Old Base Pointer"
		        "\n"    "\tmovq\t%rsp,\t%rbp\t# Save Old Stack Pointer"
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
