#ifndef __TOKENIZER__
#define __TOKENIZER__

#define BUFFER_SIZE 100

/* Unhandled composite triggers:

			other,			// idea: if the next char causes unsuccessful transition, then trigger other.

	LEFT TO DO:

			file reading
			file writing
			dividing line into buffer size
			driver function
			TESTING!
			Report
			proper error messages/handling/recovery???
*/

#include "all_includes.h"

struct Token;

class Tokenizer {
	friend class Compiler;

public:
	Tokenizer();
	~Tokenizer();
	const char* GetCurrentState();
	void GenerateTokenStream(std::string fileName);
	void OutputTokenStream(std::string fileName = "tokenstream.txt");
	void OutputErrorMessages(std::string fileName = "errors.txt");

//protected:
	enum class States { s0, //	-start
						s1, //		token: ws
						s2, //		token: unexpected
						s3, //		token: id
						s4, //		token: bop
						//s5, //		token: bop(NOT)
						//s6, //		token: bop(OR)
						s7, //		token: reserved
						/*s8, //		token: reserved(THEN)
						s9, //		token: reserved(ELSE)
						s10, //		token: reserved(FOR)
						s11, //		token: reserved(GET)
						s12, //		token: reserved(PUT)
						s13, //		token: reserved(RETURN)
						s14, //		token: reserved(PROGRAM)*/
						s15, //		token: type_decl
						//s16, //		token: type_decl(INT)
						//s17, //		token: type_decl(FLOAT)
						s18, //		token: integer
						s19, //	-integer followed by .
						s20, //		token: floating_point
						s21, // -incomplete float ending with 0
						s22, //		token: floating point
						s23, // -starting with 0
						s24, //		token: integer(0)
						s25, //		token: period
						s26, // -start with =
						s27, //		token: assign
						s28, //		token: relop(EQ)
						s29, //	-start with <
						s30, //		token: relop(LT)
						s31, //		token: relop(NE)
						s32, //		token: relop(LE)
						s33, //	-start with >
						s34, //		token: relop(GT)
						s35, //		token: relop(GE)
						s36, //		token: apostrophe
						s37, //		token: semicolon
						s38, //	-start with *
						s39, //		token: close_comment
						s40, //		token: arop(MUL)
						s41, //	-start with /
						s42, //		token: line_comment
						s43, //		token: arop(DIV)
						s44, //		token: open_comment
						s45, //		token: arop(ADD)
						s46, //		token: arop(SUB)
						s47, //		token: close_paren
						s48, //		token: open_paren
						s49, //		token: close_brace
						s50, //		token: open_brace
						s51, //		token: close_sq_bracket
						s52, //		token: open_sq_bracket
						s53,  //		token: underscore
						s54, // possible reserved bop
						s55, // possible reserved
						s56 //possible reserved type_decl
	};
	enum class Triggers {
 other,		// whatever isn't below
		whitespace,	// space, \t, \n
		letter,		// a..zA..Z
		digit,		// 1..9
		zero,			// 0
		dot,			// .
		and,			// and
		not,			// not
		or,			// or
		_if,			// if
		_then,		// then
		_else,		// else
		_for,			// for
		_get,			// get
		_put,			// put
		_return,		// return
		_program,		// program
		equal,		// =
		less_than,	// <
		greater_than,	// >
		apostrophe,	// '
		semicolon,	// ;
		star,			// *
		forwardslash,	// /
		plus,			// +
		minus,		// -
		close_paren,	// )
		open_paren,	// (
		close_brace,	// }
		open_brace,	// {
		close_sq_bracket,	// ]
		open_sq_bracket,	// [
		underscore,		// _
		_class,			// class
		_int,			// int
		_float			// float

	};

	FSM::Fsm<States, States::s0 /*Starting state*/, Triggers> fsm;
	
	std::string curr_line;
	int line_pos;
	int line_number;

	bool successful_transition;
	bool final_state;
	bool keyword_hit;

	std::vector<std::string> errorMessages;
	std::vector<Token> tokenStream;	
	Token * curr_token;
	States latestCorrectState;

	void initializeFSM();
	static const char* stringifyState(States state);
	Triggers convertCharToTrigger(char ch);
	void isKeywordHit();	
	void pushError(std::string comment);

	void pushToken();
	void newToken(States state, bool terminal_state = false, Types type = Types::none, std::string value = "");
	void updateLastToken(States state, bool terminal_state = false, Types type = Types::none, std::string value = "");
	void Tokenizer::handleReservedWords();
	void resetFSM();

	void addTransition(States from_state,					// from state
					   States to_state,						// to state
					   Triggers trigger,					// trigger
					   std::function<bool(void)> guard_func,		// guard i.e. condition to meet before transitioning
					   std::function<void(void)> action_func);	// action i.e. function to call when transitioning from A to B)
};

#endif /*__TOKENIZER__*/

