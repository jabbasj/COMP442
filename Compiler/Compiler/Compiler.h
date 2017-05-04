#ifndef __COMPILER__
#define __COMPILER__

#include "all_includes.h"

class Compiler {
public:
	Compiler();
	~Compiler();

//private: TODO: remove after done testing
	Tokenizer tokenizer;
	std::string sourceFile;
};



#endif /*__COMPILER__*/