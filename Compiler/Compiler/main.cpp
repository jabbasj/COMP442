// Compiler.cpp : Defines the entry point for the console application.
//

#include "all_includes.h"


int _tmain(int argc, _TCHAR* argv[])
{
	Compiler compiler;

	Tokenizer * tokenizer = &compiler.tokenizer;

	//tokenizer->test();

	tokenizer->GenerateTokenStream("source.txt");

	m_Pause();

	return 0;
}

void m_Print(const char msg[]) {
#ifdef VERBOSE
	std::cout << msg << std::endl;
#endif
}

void m_Pause() {
	std::cout << "Press enter to continue...";
	std::cin.get();
}

