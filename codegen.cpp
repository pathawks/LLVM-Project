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

using namespace std;
using namespace llvm;

/**
 * Get the register or address of the nth parameter passed to a function
 * according to macOS ABI
 */
string arg(unsigned n) {
	switch (n) {
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
		s << (n-4)*-8 << "(%rbp)";
		return s.str();
	}
}

/**
 * https://llvm.org/docs/LangRef.html#id293
 */
string condition(unsigned cond) {
	switch (cond) {
	case 1: 	// Equal
		return "eq";
	case 2: 	// Not Equal
		return "ne";
	case 3: 	// Unsigned Greater Than
		return "ugt";
	case 4: 	// Unsigned Greater Than or Equal
		return "uge";
	case 5: 	// Unsigned Less Than
		return "ult";
	case 6: 	// Unsigned Less Than or Equal
		return "ule";
	case 7: 	// Signed Greater Than
		return "sgt";
	case 8: 	// Signed Greater Than or Equal
		return "sge";
	case 9: 	// Signed less than
		return "slt";
	case 10:	// Signed less than or equal
		return "sle";
	default:
		return "???";
	}
}

/**
 * Escape string to be output in .s file
 */
string escape(string str) {
	size_t i=0;
	while (i<str.size()) {
		if (str[i] == '\n') {
			str.replace(i, 1, "\\n");
		} else if (str[i] == '\t') {
			str.replace(i, 1, "\\t");
		} else if (str[i] == '\"') {
			str.replace(i, 1, "\\\"");
		} else {
			++i;
		}
	}
	return str;
}

/**
 * Convert operand value to string
 */
string op(const Value *v) {
	stringstream s;
	if (const BinaryOperator* a = dyn_cast<const BinaryOperator>(v)) {
		return op(a->getOperand(0)); // HACK
	} else if (const ConstantInt* c = dyn_cast<const ConstantInt>(v)) {
		s << '$' << c->getSExtValue();
	} else if (const GlobalVariable* m = dyn_cast<const GlobalVariable>(v)) {
		if (m->hasInitializer()) {
			return op(m->getInitializer());
		} else {
			s << "GlobalVariable";
		}
	} else if (const ConstantDataArray* m = dyn_cast<const ConstantDataArray>(v)) {
		s << ".string \"" << escape(m->getAsCString().str()) << "\"";
	} else if (const ConstantAggregateZero* m = dyn_cast<const ConstantAggregateZero>(v)) {
		if (m->isNullValue()) {
			return "NULL";
		}
		for (int i=0; i<m->getNumElements(); ++i) {
			s << op(m->getAggregateElement(i));
		}
	} else if (const UndefValue* m = dyn_cast<const UndefValue>(v)) {
		s << "UndefValue";
	} else if (const ConstantData* m = dyn_cast<const ConstantData>(v)) {
		s << "ConstantData";
	} else if (const ConstantPointerNull* m = dyn_cast<const ConstantPointerNull>(v)) {
		s << "ConstantPointerNull";
	} else if (const ConstantTokenNone* m = dyn_cast<const ConstantTokenNone>(v)) {
		s << "ConstantTokenNone";
	} else if (const ConstantDataSequential* m = dyn_cast<const ConstantDataSequential>(v)) {
		s << "ConstantDataSequential";
	} else if (const ConstantExpr* m = dyn_cast<const ConstantExpr>(v)) {
		if (m->hasName()) {
			s << m->getName().str();
		} else {
			s << ".str(%rip)";
		}
	} else if (const Constant* m = dyn_cast<const Constant>(v)) {
		s << "Constant";
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
	} else if (const BasicBlock* b = dyn_cast<const BasicBlock>(v)) {
		s << "Label_" << b->getValueID();
	} else {
		cerr << "Unknown Operand";
		s    << "Unknown Operand";
	}
	return s.str();
}

/**
 * Compile LLVM Instruction to x86 Assembly
 */
string compile(Instruction &i) {
	stringstream s;
	switch (i.getOpcode()) {
	case Instruction::Alloca:
		s << "subq\t$16,\t%rsp\t# Make space on the stack";
		break;
	case Instruction::Store:
		s << "movq\t" << op(i.getOperand(0)) << ",\t%r11\t# Copy value to a temp register"
		     "\n\tmovq\t" "%r11" ",\t" << op(i.getOperand(1)) << "\t# Copy from temp register to destination"
		;
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
	case Instruction::Br:
		if (i.getNumOperands() > 1) {
			s << "jne\t" << op(i.getOperand(1));
			s << "\n\t";
			s << "jmp\t" << op(i.getOperand(2));
		} else {
			s << "jmp\t" << op(i.getOperand(0));
		}
		break;
	case Instruction::ICmp:
		s << "cmp\t" << op(i.getOperand(0)) << ",\t" << op(i.getOperand(1));
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
			cout << "\n.equ " << f.getName().str() << ", _" << f.getName().str()
			     << "\t# External function" << endl;
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
		cout << "\n" << gg->getName().str() << ": " << op(gg) << endl;
	}
}
