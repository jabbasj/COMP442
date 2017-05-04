#include "all_includes.h"


Tokenizer::Tokenizer()
{
	initializeFSM();

	line_number = 0;
	line_pos = 0;
	curr_token = new Token();

	successful_transition = false;
	final_state = false;
}

Tokenizer::~Tokenizer()
{

}


void Tokenizer::GenerateTokenStream(std::string fileName) {

	std::ifstream input_file(fileName);

	try {
		if (input_file.is_open()) {

			std::cout << "Tokenizing: " << fileName << "...\n\n";
			
			while (getline(input_file, curr_line)) {
				
				line_number++;
				std::transform(curr_line.begin(), curr_line.end(), curr_line.begin(), ::tolower);
				int attempts = curr_line.size() * 2;

				for (line_pos = 0; line_pos < curr_line.size(); line_pos++)
				{			
					final_state = false;
					keyword_hit = false;
					successful_transition = false;

					//m_Print("Current state: ");	m_Print(GetCurrentState());

					//std::cout << "Executing char: '" << curr_line[line_pos] << "'\n";

					Triggers conversion = convertCharToTrigger(curr_line[line_pos]);
					fsm.execute(conversion);

					if (keyword_hit) {
						handleReservedWords();
					}

					if (!successful_transition) {
						fsm.execute(Triggers::other);

						if (!successful_transition) {
							resetFSM();						
						}

						if (conversion != Triggers::whitespace) {
							line_pos--;							
						}

						attempts--;
						if (attempts < 0) {
							pushError("Tokenizer stuck.");
							break;
						}
					}

					if (final_state) {
						latestCorrectState = fsm.state();
					}
					//system("cls");
				}	
				if (!final_state) {
					if (fsm.state() != States::s0) {
						pushError("Non-final state.");
					}
				}

				fsm.execute(Triggers::other);
				resetFSM();
			}

			input_file.close();
			OutputTokenStream();
			std::cout << std::endl;
			OutputErrorMessages();
		}
		else {
			pushError("Input file (" + fileName + ") not found!");
		}
	}
	catch (...) {
		pushError("EXCEPTION: GenerateTokenStream()!");
	}
}


//Adds all the possible transitions in the state diagram
void Tokenizer::initializeFSM()
{
	//from state, to state, trigger, guard, action

	//start to ws (final)
	addTransition(States::s0, States::s1, Triggers::whitespace, [&]{return true; }, [&]{newToken(States::s1, true); });

	//ws (final) to ws (final)
	addTransition(States::s1, States::s1, Triggers::whitespace, [&]{return true; }, [&]{updateLastToken(States::s1, true); });

	//ws (final) to start
	addTransition(States::s1, States::s0, Triggers::other, [&]{return true; }, [&]{successful_transition = true; });

	//start to unexpected (final)
	addTransition(States::s0, States::s2, Triggers::other, [&]{return true; }, [&]{newToken(States::s2, true); });

	//unexpected (final) to start
	addTransition(States::s2, States::s0, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s2); successful_transition = false; });

	//start to id (final)
	addTransition(States::s0, States::s3, Triggers::letter, [&]{return true; }, [&]{newToken(States::s3, true, Types::string); });

	//id (final) to id (final) 
	addTransition(States::s3, States::s3, Triggers::letter, [&]{return true; }, [&]{updateLastToken(States::s3, true); isKeywordHit(); });
	addTransition(States::s3, States::s3, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s3, true); });
	addTransition(States::s3, States::s3, Triggers::underscore, [&]{return true; }, [&]{updateLastToken(States::s3, true);  });

	//id (final) to 54
	addTransition(States::s3, States::s54, Triggers::and, [&]{return true; }, [&]{updateLastToken(States::s54, false);  });
	addTransition(States::s3, States::s54, Triggers::not, [&]{return true; }, [&]{updateLastToken(States::s54, false);  });
	addTransition(States::s3, States::s54, Triggers::or, [&]{return true; }, [&]{updateLastToken(States::s54, false);  });

	//54 back to id (final)
	addTransition(States::s54, States::s3, Triggers::letter, [&]{return !keyword_hit; }, [&]{updateLastToken(States::s3);  });
	addTransition(States::s54, States::s3, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s3);  });
	addTransition(States::s54, States::s3, Triggers::underscore, [&]{return true; }, [&]{updateLastToken(States::s3);  });
	
	//id (final) to bop(and) (final)
	addTransition(States::s54, States::s4, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s4, true, Types::bop);  });

	//54 to bop(not) (final)
	//addTransition(States::s54, States::s5, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s5, true, Types::bop); });

	//54 to bop(or) (final)
	//addTransition(States::s54, States::s6, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s6, true, Types::bop);  });

	// id (final) to 55
	addTransition(States::s3, States::s55, Triggers::_if, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_then, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_else, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_for, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_get, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_put, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_return, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });
	addTransition(States::s3, States::s55, Triggers::_program, [&]{return true; }, [&]{updateLastToken(States::s55, false);  });

	//55 back to id (final)
	addTransition(States::s55, States::s3, Triggers::letter, [&]{return !keyword_hit; }, [&]{updateLastToken(States::s3, true); });
	addTransition(States::s55, States::s3, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s3, true); });
	addTransition(States::s55, States::s3, Triggers::underscore, [&]{return true; }, [&]{updateLastToken(States::s3, true); });

	//55 to reserved (final)
	addTransition(States::s55, States::s7, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s7, true, Types::reserved); });

	
	// id (final) to 56
	addTransition(States::s3, States::s56, Triggers::_class, [&]{return true; }, [&]{updateLastToken(States::s56, false);  });
	addTransition(States::s3, States::s56, Triggers::_int, [&]{return true; }, [&]{updateLastToken(States::s56, false);  });
	addTransition(States::s3, States::s56, Triggers::_float, [&]{return true; }, [&]{updateLastToken(States::s56, false); });

	// 56 back to id (final)
	addTransition(States::s56, States::s3, Triggers::letter, [&]{return !keyword_hit; }, [&]{updateLastToken(States::s3, true); });
	addTransition(States::s56, States::s3, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s3, true); });
	addTransition(States::s56, States::s3, Triggers::underscore, [&]{return true; }, [&]{updateLastToken(States::s3, true); });

	// 56 to type_decl(class) (class)
	addTransition(States::s56, States::s15, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s15, true, Types::type_decl);  });

	// 56 to type_decl(int) (final)
	//addTransition(States::s56, States::s16, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s16, true, Types::type_decl);  });

	// 56 to type_decl(float) (final)
	//addTransition(States::s56, States::s17, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s17, true, Types::type_decl);  });

	// start to integer (final)
	addTransition(States::s0, States::s18, Triggers::digit, [&]{return true; }, [&]{newToken(States::s18, true, Types::integer); });

	// integer (final) to integer (final)
	addTransition(States::s18, States::s18, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s18, true); });
	addTransition(States::s18, States::s18, Triggers::zero, [&]{return true; }, [&]{updateLastToken(States::s18, true); });

	// integer (final) to 19
	addTransition(States::s18, States::s19, Triggers::dot, [&]{return true; }, [&]{updateLastToken(States::s19, false, Types::floatingpoint); });

	// 19 to floating_point (final)
	addTransition(States::s19, States::s20, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s20, true); });

	// 19 to 21
	addTransition(States::s19, States::s21, Triggers::zero, [&]{return true; }, [&]{updateLastToken(States::s21); });

	// floating_point (final) to floating_point (final)
	addTransition(States::s20, States::s20, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s20, true); });

	// floating_point (final) to 21
	addTransition(States::s20, States::s21, Triggers::zero, [&]{return true; }, [&]{updateLastToken(States::s21); });

	// 21 to 21
	addTransition(States::s21, States::s21, Triggers::zero, [&]{return true; }, [&]{updateLastToken(States::s21); });

	// 21 to floating_point(20) (final)
	addTransition(States::s21, States::s20, Triggers::digit, [&]{return true; }, [&]{updateLastToken(States::s20, true); });

	//21 to floating_point(22) (final)
	addTransition(States::s21, States::s22, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s22, true);  });

	//start to 23
	addTransition(States::s0, States::s23, Triggers::zero, [&]{return true; }, [&]{newToken(States::s23, false, Types::integer); });

	//23 to integer (final)
	addTransition(States::s23, States::s24, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s24, true);  });

	//23 to 19
	addTransition(States::s23, States::s19, Triggers::dot, [&]{return true; }, [&]{updateLastToken(States::s19, false, Types::floatingpoint); });

	//start to 25 (final)
	addTransition(States::s0, States::s25, Triggers::dot, [&]{return true; }, [&]{newToken(States::s25, true);  });

	//start to 26
	addTransition(States::s0, States::s26, Triggers::equal, [&]{return true; }, [&]{newToken(States::s26); });

	//26 to assign (final)
	addTransition(States::s26, States::s27, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s27, true);  });

	//26 to relop(EQ) (final)
	addTransition(States::s26, States::s28, Triggers::equal, [&]{return true; }, [&]{updateLastToken(States::s28, true, Types::relop, "EQ");  });

	//start to 29
	addTransition(States::s0, States::s29, Triggers::less_than, [&]{return true; }, [&]{newToken(States::s29); });

	//29 to relop(LT) (final)
	addTransition(States::s29, States::s30, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s30, true, Types::relop, "LT");  });

	//29 to relop(NE) (final)
	addTransition(States::s29, States::s31, Triggers::greater_than, [&]{return true; }, [&]{updateLastToken(States::s31, true, Types::relop, "NE");  });

	//29 to relop(LE) (final)
	addTransition(States::s29, States::s32, Triggers::equal, [&]{return true; }, [&]{updateLastToken(States::s32, true, Types::relop, "LE");  });

	//start to 33
	addTransition(States::s0, States::s33, Triggers::greater_than, [&]{return true; }, [&]{newToken(States::s33); });

	//33 to relop(GT) (final)
	addTransition(States::s33, States::s34, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s34, true, Types::relop, "GT");  });

	//33 to relop(GE) (final)
	addTransition(States::s33, States::s35, Triggers::equal, [&]{return true; }, [&]{updateLastToken(States::s35, true, Types::relop, "GE");  });

	//start to apostrophe (final)
	addTransition(States::s0, States::s36, Triggers::apostrophe, [&]{return true; }, [&]{newToken(States::s36, true);  });

	//start to semicolon (final)
	addTransition(States::s0, States::s37, Triggers::semicolon, [&]{return true; }, [&]{newToken(States::s37, true);  });

	//start to 38
	addTransition(States::s0, States::s38, Triggers::star, [&]{return true; }, [&]{newToken(States::s38); });

	//38 to close_comment (final)
	addTransition(States::s38, States::s39, Triggers::forwardslash, [&]{return true; }, [&]{updateLastToken(States::s39, true);  });

	//38 to arop(MUL) (final)
	addTransition(States::s38, States::s40, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s40, true, Types::arop, "MUL");  });

	//start to 41
	addTransition(States::s0, States::s41, Triggers::forwardslash, [&]{return true; }, [&]{newToken(States::s41); });

	//41 to line_comment (final)
	addTransition(States::s41, States::s42, Triggers::forwardslash, [&]{return true; }, [&]{updateLastToken(States::s42, true);  });

	//41 to arop(DIV) (final)
	addTransition(States::s41, States::s43, Triggers::other, [&]{return true; }, [&]{updateLastToken(States::s43, true, Types::arop, "DIV");  });

	//41 to open_comment (final)
	addTransition(States::s41, States::s44, Triggers::star, [&]{return true; }, [&]{updateLastToken(States::s44, true);  });

	//start to arop(ADD) (final)
	addTransition(States::s0, States::s45, Triggers::plus, [&]{return true; }, [&]{newToken(States::s45, true, Types::arop, "ADD");  });

	//start to arop(SUB) (final)
	addTransition(States::s0, States::s46, Triggers::minus, [&]{return true; }, [&]{newToken(States::s46, true, Types::arop, "SUB");  });

	//start to close_paren
	addTransition(States::s0, States::s47, Triggers::close_paren, [&]{return true; }, [&]{newToken(States::s47, true);  });

	//start to open_paren
	addTransition(States::s0, States::s48, Triggers::open_paren, [&]{return true; }, [&]{newToken(States::s48, true);  });

	//start to close_brace
	addTransition(States::s0, States::s49, Triggers::close_brace, [&]{return true; }, [&]{newToken(States::s49, true);  });

	//start to open_brace
	addTransition(States::s0, States::s50, Triggers::open_brace, [&]{return true; }, [&]{newToken(States::s50, true);  });

	//start to close_sq_bracket
	addTransition(States::s0, States::s51, Triggers::close_sq_bracket, [&]{return true; }, [&]{newToken(States::s51, true);  });

	//start to open_sq_bracket
	addTransition(States::s0, States::s52, Triggers::open_sq_bracket, [&]{return true; }, [&]{newToken(States::s52, true);  });

	//start to underscore
	addTransition(States::s0, States::s53, Triggers::underscore, [&]{return true; }, [&]{newToken(States::s53, true);  });
}


void Tokenizer::newToken(States state, bool terminal_state /* = false*/, Types type /* = none */, std::string value /*= ""*/) {
	
	curr_token = new Token(state, type, value);

	if (value == "") {
		curr_token->value = curr_line[line_pos];
	}

	final_state = terminal_state;
	successful_transition = true;
}



void Tokenizer::updateLastToken(States state, bool terminal_state /* = false*/, Types type /* = none*/, std::string value /*= ""*/) {
	
	curr_token->state = state;
	curr_token->name = stringifyState(state);

	if (type != Types::none) {
		curr_token->type = type;
	}

	if (value == "") {
		if (!keyword_hit) {
			curr_token->value = curr_token->value + curr_line[line_pos];
		}
	}
	else {
		curr_token->value = value;
	}

	final_state = terminal_state;
	successful_transition = true;
}

void Tokenizer::handleReservedWords() {

	std::string id = curr_token->value;

	if (id == "and") { fsm.execute(Triggers::and); return; }
	if (id == "not") { fsm.execute(Triggers::not); return; }
	if (id == "or") { fsm.execute(Triggers::or); return; }
	if (id == "if") { fsm.execute(Triggers::_if); return; }
	if (id == "then") { fsm.execute(Triggers::_then); return; }
	if (id == "else") { fsm.execute(Triggers::_else); return; }
	if (id == "for") { fsm.execute(Triggers::_for);  return; }
	if (id == "get") { fsm.execute(Triggers::_get); return; }
	if (id == "put") { fsm.execute(Triggers::_put); return; }
	if (id == "return") { fsm.execute(Triggers::_return); return; }
	if (id == "program") { fsm.execute(Triggers::_program); return; }
	if (id == "class") { fsm.execute(Triggers::_class); return; }
	if (id == "int") { fsm.execute(Triggers::_int); return; }
	if (id == "float") { fsm.execute(Triggers::_float); return; }
}

void Tokenizer::isKeywordHit() {

	std::string id = curr_token->value;

	if (id == "and" || id == "not" || id == "or"
		|| id == "if" || id == "then" || id == "else" || id == "for"
		|| id == "get" || id == "put" || id == "return" || id == "program" || id == "class" ||
		id == "int" || id == "float") {
		
		keyword_hit = true;
	}
}

void Tokenizer::pushToken() {

	if (curr_token->state != States::s0) {
		tokenStream.push_back(*curr_token);
	}
}

void Tokenizer::resetFSM() {

	pushToken();
	fsm.reset();

	curr_token->state = fsm.state();
}

/*
States from_state,					// from state
States to_state,					// to state
Triggers trigger,					// trigger
std::function<bool> guard_func,		// guard i.e. condition to meet before transitioning
std::function<void> action_func);	// action i.e. function to call when transitioning from A to B)
*/
void Tokenizer::addTransition(States from_state, States to_state, Triggers trigger, std::function<bool(void)> guard_func, std::function<void(void)> action_func) {
	fsm.add_transitions({
		{
			from_state,					// from state
			to_state,					// to state
			trigger,					// triggers
			guard_func,					// guard i.e. condition to meet before transitioning
			action_func					// action i.e. function to call when transitioning from A to B
		},
	});
}



//Returns the current state of the fsm in readable form
const char* Tokenizer::GetCurrentState() {
	return stringifyState(fsm.state());
}



Tokenizer::Triggers Tokenizer::convertCharToTrigger(char ch) {

	std::string temp = ""; temp += ch;
	std::regex letter("([a-zA-Z])");
	std::regex digit("([1-9])");
	std::smatch result;
	bool matched = false;
	
	matched = std::regex_match(temp, result, letter);

	if (matched) {
		return Triggers::letter;
	}

	matched = std::regex_match(temp, result, digit);

	if (matched) {
		return Triggers::digit;
	}	

	switch (ch) {
		case '0':
			return Triggers::zero;
		case '\t':
			return Triggers::whitespace;
		case '\n':
			return Triggers::whitespace;
		case ' ':
			return Triggers::whitespace;
		case '.':
			return Triggers::dot;
		case '=':
			return Triggers::equal;
		case '<':
			return Triggers::less_than;
		case '>':
			return Triggers::greater_than;
		case '\'':
			return Triggers::apostrophe;
		case ';':
			return Triggers::semicolon;
		case '*':
			return Triggers::star;
		case '/':
			return Triggers::forwardslash;
		case '+':
			return Triggers::plus;
		case '-':
			return Triggers::minus;
		case ')':
			return Triggers::close_paren;
		case '(':
			return Triggers::open_paren;
		case '}':
			return Triggers::close_brace;
		case '{':
			return Triggers::open_brace;
		case ']':
			return Triggers::close_sq_bracket;
		case '[':
			return Triggers::open_sq_bracket;
		case '_':
			return Triggers::underscore;
	default:
		return Triggers::other;
	}
}

//Converts state into its human-readable/printable form
const char* Tokenizer::stringifyState(States state) {
	switch (state) {
		case States::s0:
			return "state0";
		case States::s1:
			return "ws";
		case States::s2:
			return "unexpected";
		case States::s3:
			return "id";
		case States::s4:
			return "bop";
			/*
		case States::s5:
			return "bop(NOT)";
		case States::s6:
			return "bop(OR)";*/
		case States::s7:
			return "reserved"; /*
		case States::s8:
			return "reserved(THEN)";
		case States::s9:
			return "reserved(ELSE)";
		case States::s10:
			return "reserved(FOR)";
		case States::s11:
			return "reserved(GET)";
		case States::s12:
			return "reserved(PUT)";
		case States::s13:
			return "reserved(RETURN)";
		case States::s14:
			return "reserved(PROGRAM)";*/
		case States::s15:
			return "type_decl"; /*
		case States::s16:
			return "type_decl(INT)";
		case States::s17:
			return "type_decl(FLOAT)";*/
		case States::s18:
			return "integer";
		case States::s19:
			return "state19";
		case States::s20:
			return "floating_point";
		case States::s21:
			return "state21";
		case States::s22:
			return "floating_point";
		case States::s23:
			return "state23";
		case States::s24:
			return "integer";
		case States::s25:
			return "period";
		case States::s26:
			return "state26";
		case States::s27:
			return "assign";
		case States::s28:
			return "relop(EQ)";
		case States::s29:
			return "state29";
		case States::s30:
			return "relop(LT)";
		case States::s31:
			return "relop(NE)";
		case States::s32:
			return "relop(LE)";
		case States::s33:
			return "state33";
		case States::s34:
			return "relop(GT)";
		case States::s35:
			return "relop(GE)";
		case States::s36:
			return "apostrophe";
		case States::s37:
			return "semicolon";
		case States::s38:
			return "state38";
		case States::s39:
			return "close_comment";
		case States::s40:
			return "arop(MUL)";
		case States::s41:
			return "state41";
		case States::s42:
			return "line_comment";
		case States::s43:
			return "arop(DIV)";
		case States::s44:
			return "open_comment";
		case States::s45:
			return "arop(ADD)";
		case States::s46:
			return "arop(SUB)";
		case States::s47:
			return "close_paren";
		case States::s48:
			return "open_paren";
		case States::s49:
			return "close_brace";
		case States::s50:
			return "open_brace";
		case States::s51:
			return "close_sq_bracket";
		case States::s52:
			return "open_sq_bracket";
		case States::s53:
			return "underscore";
		case States::s54:
			return "state54";
		case States::s55:
			return "state55";
		case States::s56:
			return "state56";
	default:
		return "UNKNOWN";
	}		
}


void Tokenizer::OutputTokenStream(std::string fileName) {

	std::ofstream output_file(fileName);

	std::cout << "Token stream generated:\n\n";

	for (int i = 0; i < tokenStream.size(); i++) {

		std::string out_line = tokenStream[i].name;

		if (tokenStream[i].type == Types::bop || tokenStream[i].type == Types::reserved || tokenStream[i].type == Types::type_decl) {
			
			tokenStream[i].value = std::string(tokenStream[i].value.begin(), tokenStream[i].value.end() - 1);
			
			out_line += "(" + tokenStream[i].value + ")";
		}

		out_line += " ";

		std::cout << out_line;
		output_file << out_line;
	}

	std::cout << "\n";

	output_file.close();
}

void Tokenizer::pushError(std::string comment) {
	std::string error_msg;

	error_msg += "Invalid token: ";
	error_msg += GetCurrentState();
	error_msg += " at line: " + std::to_string(line_number);
	error_msg += " loc: " + std::to_string(line_pos) + ".";
	error_msg += " Last safe state: ";
	error_msg += stringifyState(latestCorrectState);
	error_msg += ". ";
	error_msg += comment + " ";

	switch (fsm.state()) {
	case States::s2:
		error_msg += "Unexpected symbol";
		break;
	case States::s54:
		error_msg += "Identifier resembles reserved words (and | not | or)";
		break;
	case States::s55:
		error_msg += "Identifier resembles reserved words (if | then | else | for | get | put | return | program)";
		break;
	case States::s56:
		error_msg += "Identifier resembles reserved words (class | int | float)";
		break;
	case States::s19:
		error_msg += "Integer followed by dot but no other numbers";
		break;
	case States::s21:
		error_msg += "Float ending with zeros";
		break;
	case States::s23:
		error_msg += "'0' floating";
		break;
	case States::s26:
		error_msg += "Tokenizer stuck at =";
		break;
	case States::s29:
		error_msg += "Tokenizer stuck at <";
		break;
	case States::s33:
		error_msg += "Tokenizer stuck at >";
		break;
	case States::s41:
		error_msg += "Cannot resolve forward slash";
		break;
	default:
		error_msg += "Unknown cause. Please report it.";
	}
	error_msg += ".";

	errorMessages.push_back(error_msg);
}

void Tokenizer::OutputErrorMessages(std::string fileName /*= "errors.txt"*/) {

	std::ofstream output_file(fileName);

	if (errorMessages.size() > 0) {
		std::cout << "\n\nErrors detected:\n\n";

		for (int i = 0; i < errorMessages.size(); i++) {

			std::string out_line = errorMessages[i];

			std::cout << out_line << "\n\n";
			output_file << out_line << "\n\n";;
		}
	}

	output_file.close();
}

