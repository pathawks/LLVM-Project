/**
 * Compile LLVM Instruction to x86 Assembly
 */

#include "compile.hpp"
#include "stack.hpp"
#include "label.hpp"
#include "op.hpp"

using namespace std;
using namespace llvm;

string compile(Instruction &i) {
	stringstream s;
	switch (i.getOpcode()) {
	case Instruction::Alloca:
		return "";
		s << "subq\t$8,\t%rsp\t# Make space on the stack";
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
			Value *operand = i.getOperand(j);
			if (const LoadInst* load = dyn_cast<const LoadInst>(operand)) {
				s << "movq\t" << op(operand) << ",\t" << arg(j) << "\n\t";
			} else {
				s << "leaq\t" << op(operand) << ",\t" << arg(j) << "\n\t";
			}
		}
		s << "andq\t$~15,\t%rsp\n\t"
		     "callq\t" << op(i.getOperand(i.getNumOperands()-1));
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
		s << "\n\taddq\t" << op(i.getOperand(1)) << ",\t%r11";
		s << "\n\tmovq\t%r11,\t" << getStackPosition(&i);
		break;
	case Instruction::Br:
		if (i.getNumOperands() > 1) {
			Value *operand = i.getOperand(0);
			if (const Instruction* cmp = dyn_cast<const Instruction>(operand)) {
				s << "cmpq\t$0,\t" << getStackPosition(cmp) << "\n\t";
				s << "jnz\t" << op(i.getOperand(1));
				s << "\n\t";
				s << "jmp\t" << op(i.getOperand(2));
			} else {
				s << "BAD CAST IN BR";
			}
		} else {
			s << "jmp\t" << op(i.getOperand(0));
		}
		break;
	case Instruction::ICmp:
		const ICmpInst* cmpi = dyn_cast<const ICmpInst>(&i);
		Value *a = (Value*)((size_t)(&i) + 1);
		Value *b = (Value*)((size_t)(&i) + 2);
		string ifTrue      = getLabel(a, "ConditionTrue_");
		string fallThrough = getLabel(b, "ConditionDone_");
		s << "movq\t" << op(i.getOperand(0)) << ",\t%r11";
		s << "\n\tsubq\t" << op(i.getOperand(1)) << ",\t%r11";
		s << "\n\t" << condition(cmpi->getInversePredicate()) << "\t" << ifTrue;
		s << "\n\tmovq\t$FALSE,\t" << getStackPosition(&i);
		s << "\n\tjmp\t" << fallThrough;
		s << "\n" << ifTrue << ":";
		s << "\n\tmovq\t$TRUE,\t" << getStackPosition(&i);
		s << "\n" << fallThrough << ":";
		break;
	}
	s << '\n';
	return s.str();
}
