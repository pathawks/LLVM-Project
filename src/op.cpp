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

#include "constants.hpp"
#include "stack.hpp"
#include "label.hpp"
#include "op.hpp"

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
	case llvm::CmpInst::ICMP_EQ: 	// Equal
		return "je";
	case llvm::CmpInst::ICMP_NE: 	// Not Equal
		return "jne";
	case llvm::CmpInst::ICMP_UGT: 	// Unsigned Greater Than
		return "jnbe";
	case llvm::CmpInst::ICMP_UGE: 	// Unsigned Greater Than or Equal
		return "jnb";
	case llvm::CmpInst::ICMP_ULT: 	// Unsigned Less Than
		return "jnae";
	case llvm::CmpInst::ICMP_ULE: 	// Unsigned Less Than or Equal
		return "jna";
	case llvm::CmpInst::ICMP_SGT: 	// Signed Greater Than
		return "jnle";
	case llvm::CmpInst::ICMP_SGE: 	// Signed Greater Than or Equal
		return "jnl";
	case llvm::CmpInst::ICMP_SLT: 	// Signed Less Than
		return "jnge";
	case llvm::CmpInst::ICMP_SLE: 	// Signed Less Than or Equal
		return "jng";
	default:
		return "???";
	}
}

/**
 * Convert operand value to string
 */
string op(const Value *v) {
	stringstream s;
	if (const BinaryOperator* i = dyn_cast<const BinaryOperator>(v)) {
		return getStackPosition(i);
	} else if (const ConstantInt* c = dyn_cast<const ConstantInt>(v)) {
		s << '$' << c->getSExtValue();
	} else if (const GlobalVariable* m = dyn_cast<const GlobalVariable>(v)) {
		s << m->getName().str() << "(%rip)";
	} else if (const ConstantExpr* m = dyn_cast<const ConstantExpr>(v)) {
		s << m->getOperand(0)->getName().str() << "(%rip)";
	} else if (const ConstantData* m = dyn_cast<const ConstantData>(v)) {
		s << m->getName().str() << "(%rip)";
	} else if (const Constant* m = dyn_cast<const Constant>(v)) {
		return v->getName().str();
	} else if (const Instruction* m = dyn_cast<const Instruction>(v)) {
		return getStackPosition(m);
	} else if (const Argument* a = dyn_cast<const Argument>(v)) {
		return arg(a->getArgNo());
	} else if (const User* m = dyn_cast<const User>(v)) {
		return "User";
	} else if (const BasicBlock* b = dyn_cast<const BasicBlock>(v)) {
		return getLabel(*b);
	} else {
		cerr << "Unknown Operand";
		return  "Unknown Operand";
	}
	return s.str();
}
