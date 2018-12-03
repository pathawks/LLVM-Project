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
		} else if (str[i] == '\0') {
			str.replace(i, 1, "\\0");
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
		return getStackPosition(a);
	} else if (const ConstantInt* c = dyn_cast<const ConstantInt>(v)) {
		s << '$' << c->getSExtValue();
	} else if (const GlobalVariable* m = dyn_cast<const GlobalVariable>(v)) {
		if (m->hasInitializer()) {
			return op(m->getInitializer());
		} else {
			s << "GlobalVariable";
		}
	} else if (const ConstantDataSequential* m = dyn_cast<const ConstantDataSequential>(v)) {
		if (m->isString()) {
			s << ".ascii \"" << escape(m->getAsString()) << "\"";
		} else {
			s << ".ascii \"Not a string\"";
		}
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
		s << m->getOperand(0)->getName().str() << "(%rip)";
	} else if (const Constant* m = dyn_cast<const Constant>(v)) {
		s << m->getName().str();
	} else if (const AllocaInst* a = dyn_cast<const AllocaInst>(v)) {
		return getStackPosition(a);
	} else if (const CallInst* a = dyn_cast<const CallInst>(v)) {
		s << "%rax";
	} else if (const SelectInst* a = dyn_cast<const SelectInst>(v)) {
		s << "SelectInst";
	} else if (const ReturnInst* a = dyn_cast<const ReturnInst>(v)) {
		s << "ReturnInst";
	} else if (const UnaryInstruction* a = dyn_cast<const UnaryInstruction>(v)) {
		s << op(a->getOperand(0)); // HACK
	} else if (const Instruction* m = dyn_cast<const Instruction>(v)) {
		s << getStackPosition(m);
	} else if (const Argument* a = dyn_cast<const Argument>(v)) {
		s << arg(a->getArgNo());
	} else if (const User* m = dyn_cast<const User>(v)) {
		s << "User";
	} else if (const BasicBlock* b = dyn_cast<const BasicBlock>(v)) {
		return getLabel(*b);
	} else {
		cerr << "Unknown Operand";
		s    << "Unknown Operand";
	}
	return s.str();
}
