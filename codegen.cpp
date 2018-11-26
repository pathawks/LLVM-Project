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

string op(const Value *v) {
	stringstream s;
	if (const BinaryOperator* a = dyn_cast<const BinaryOperator>(v)) {
		return op(a->getOperand(0)); // HACK
	} else if (const ConstantInt* c = dyn_cast<const ConstantInt>(v)) {
		s << '$' << c->getSExtValue();
	} else if (const ConstantData* m = dyn_cast<const ConstantData>(v)) {
		s << "ConstantData";
	} else if (const Constant* m = dyn_cast<const Constant>(v)) {
		if (m->hasName()) {
			s << m->getName().str();
		} else {
			s << ".str(%rip)";
		}
	} else if (const AllocaInst* a = dyn_cast<const AllocaInst>(v)) {
		if (const ConstantInt* c = dyn_cast<const ConstantInt>(a->getOperand(0))) {
			s << c->getSExtValue() << "(%rsp)";
		} else {
			s << op(a->getOperand(0)) << "()";
		}
	} else if (const CallInst* a = dyn_cast<const CallInst>(v)) {
		s << "%rax";
	} else if (const SelectInst* a = dyn_cast<const SelectInst>(v)) {
		s << "SelectInst";
	} else if (const ReturnInst* a = dyn_cast<const ReturnInst>(v)) {
		s << "ReturnInst";
	} else if (const UnaryInstruction* a = dyn_cast<const UnaryInstruction>(v)) {
		return op(a->getOperand(0)); // HACK
	} else if (const Instruction* m = dyn_cast<const Instruction>(v)) {
		s << "Inst";
	} else if (const Argument* a = dyn_cast<const Argument>(v)) {
		s << arg(a->getArgNo());
	} else if (const User* m = dyn_cast<const User>(v)) {
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
		s << "subq\t$8,\t%rsp"
		     "\n\t" "andq\t$-15,\t%rsp";
		break;
	case Instruction::Store:
		s << "movq\t" << op(i.getOperand(0)) << ",\t%r11";
		s << "\n\tmovq\t" "%r11" ",\t" << op(i.getOperand(1));
		break;
	case Instruction::Load:
		s << "movq\t" << op(i.getOperand(0)) << ",\t%r11";
		break;
	case Instruction::Call:
		for (int j=0; j<i.getNumOperands()-1; ++j) {
			s << "leaq\t" << op(i.getOperand(j)) << ",\t" << arg(j) << "\n\t";
		}
		s << "callq\t" << op(i.getOperand(i.getNumOperands()-1));
		break;
	case Instruction::Ret:
		s <<     "movq\t" << op(i.getOperand(0)) << ",\t%rax\t# Set return value"
		     "\n\tmovq\t%rbp,\t%rsp\t# Restore Old Stack Pointer"
		     "\n\tpopq\t%rbp    \t# Restore Old Base Pointer"
		     "\n\tretq            \t# Return from function"
		;
		break;
	case Instruction::Add:
		s << "movq\t" << op(i.getOperand(0)) << ",\t%r11";
		s << "\n\taddq\t%r11,\t" << op(i.getOperand(1));
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
	std::unique_ptr<Module> mod;
	if (argc > 1 && strcmp(argv[1],"-")) {
		string inputFile = argv[1];
		mod = parseIRFile(inputFile, error, context, false);
	} else {
		cerr << "No input file specified"    "\n"
			    "Reading LLVM-IR from stdin" << endl;
		mod = parseIRFile("-", error, context, false);
	}
	Module *m = mod.get();

	cout <<         "# Platform: " << m->getTargetTriple()
	     << "\n"    "# Instructions: " << m->getInstructionCount()
	     << "\n"    "# Id: " << m->getModuleIdentifier()
	     << "\n"    "# Source File: " << m->getSourceFileName()
	     << "\n"    ".global _main"
	     << "\n"    ".equ _main, main"
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
				outs() << "\n\t#\t" << instruction << "\n\t" << compile(instruction);
			}
			outs().flush();
		}
	}

	for (const GlobalVariable &g : m->globals()) {
		const Value * gg = dyn_cast<const Value>(&g);
		cout << "\n" << op(gg) << ": .string \"This is a global variable?\"" << endl;
	}
}
