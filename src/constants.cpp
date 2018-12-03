/**
 * Constants -> Literal Values
 */

#include <iostream>
#include <sstream>
#include <string>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

using namespace std;
using namespace llvm;

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
string valueToLiteral(const Value *v) {
	stringstream s;
	if (const ConstantInt* c = dyn_cast<const ConstantInt>(v)) {
		s << ".quad " << c->getSExtValue();
	} else if (const ConstantDataSequential* m = dyn_cast<const ConstantDataSequential>(v)) {
		if (m->isString()) {
			s << ".ascii \"" << escape(m->getAsString()) << "\"";
		} else {
			for (int i=0; i<m->getNumElements(); ++i) {
				s << "\n\t" << valueToLiteral(m->getAggregateElement(i));
			}
		}
	} else if (const ConstantAggregateZero* m = dyn_cast<const ConstantAggregateZero>(v)) {
		if (m->isNullValue()) {
			return "NULL";
		}
		for (int i=0; i<m->getNumElements(); ++i) {
			s << "\n\t" << valueToLiteral(m->getAggregateElement(i));
		}
	} else if (const Constant* m = dyn_cast<const Constant>(v)) {
		return valueToLiteral(m->getOperand(0));
	} else {
		cerr << "Unknown Constant";
		s    << "Unknown Constant";
	}
	return s.str();
}
