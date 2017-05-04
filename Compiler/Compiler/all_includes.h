#ifndef __ALL_INCLUDES__
#define __ALL_INCLUDES__

#define VERBOSE			// print to console or not with m_Print
enum class Types { integer, string, floatingpoint, reserved, type_decl, bop, relop, arop, none };

#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <regex>
#include <fstream>
#include <vector>
#include <algorithm>


#include "fsm.h"
#include "Tokenizer.h"
#include "Compiler.h"


//*****************************************************
//			 COMMON HELPER FUNCTION
//*****************************************************
void m_Print(const char msg[]);
void m_Pause();


struct Token {

	Token(Tokenizer::States _state, Types _type = Types::none, std::string _value = "") {

		state = _state;
		type = _type;
		value = _value;
		name = Tokenizer::stringifyState(_state);

	}

	Token() {
		state = Tokenizer::States::s0;
		name = Tokenizer::stringifyState(state);
		type = Types::none;
		value = "";
	}

	Tokenizer::States state;
	std::string name;
	Types type;
	std::string value;
};


#endif /*__ALL_INCLUDES__*/