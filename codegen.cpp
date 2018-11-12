/**
 * CodeGen Redux
 */

#include <iostream>
#include <fstream>

#include <llvm/LinkAllIR.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>

using namespace std;
using namespace llvm;

istream& in = cin;
ostream& out = cout;

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

Module* open() {
	SMDiagnostic error;
	LLVMContext context;
	return parseIRFile("HelloWorld.ll", error, context).get();
}

int main(int argc, char** argv) {
	Module *Mod = open();
	// for (Function &f : *Mod) {
		// cout << "asda" << f.getName().str() << endl;
	// }
	cout << "Name: " << Mod->getDataLayoutStr() << endl;
	Mod->materializeAll();
// 	cout << "Sieze: " << Mod->globals().size() << endl;
	for (auto &g : Mod->globals()) {
		cout << "global" << endl;
	}
	Codegen c;
	cout << c.instruction("retq") << endl;
	cout << c.instruction("push", "$1") << endl;
	cout << c.instruction("movq", "%rbp", "%rsp") << endl;
}
