/**
 * Functions related to operands
 */

std::string arg(unsigned n);
std::string condition(unsigned cond);
std::string escape(std::string str);
std::string op(const llvm::Value *v);
