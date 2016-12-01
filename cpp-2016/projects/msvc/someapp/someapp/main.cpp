#include <iostream>
#include <string>
#include "../../../../include/sjk/sjk_strings.h"
#include "../../../../include/sjk/sjk_exception.h"
#include <map>

using std::count;
using std::endl;
using std::cout;

using cmd_map_t = std::map<std::string, std::string>;
using type_map_t = std::map<std::string, std::string>;



std::string vb() {
	return "Dim Mystr as string\n Mystr = \"Hello World\" ";
}

void pop_cmd_map(cmd_map_t& map) {
	map["dim "] = "dim ";
	map[" as "] = " as ";
	
}

void pop_type_map(type_map_t& map) 
{
	map["int"] = "integer";
	
}

using namespace sjk::str;
cmd_map_t cmd_map;
type_map_t type_map;

enum class parse_state { ERR = -1, NONE, DECLARING, DECLARING_AS, VAR_NAME };

parse_state parse(int line_num, const vec_t& words,  parse_state state)
{
	switch (state)
	{
	case parse_state::NONE:
	{
		// Dim x as integer
		if (words.size() < 2) {
			if (words.size()) {
				SJK_EXCEPT("Syntax error: not enough content on line ", line_num);
			}
			else {
				SJK_EXCEPT("Syntax error: not enough content on line ", line_num, " after " , words[0]);
			}
		}
		else {
			state = parse_state::DECLARING;
			auto word = words[0];
			auto it = cmd_map.find(word + " ");
			if (it == cmd_map.end()) {
				SJK_EXCEPT("Syntax error on line ", line_num, ". Cannot understand ", word);
			}
			else {
				if (word == "dim") {
					if (words.size() != 4) {
						SJK_EXCEPT("Syntax error on line", line_num, ".Expected declaration, like: Dim i as Integer.");
					}
					word = words[1]; // TODO: check variable name is OK
					word = words[2];
					state = parse_state::DECLARING;
					if (word != "as") {
						SJK_EXCEPT("Syntax error on line", line_num, ".Expected 'as' to follow variable name.");
					}
					word = words[4];
					auto itt = type_map.find(word);
					if (itt == type_map.end()) {
						SJK_EXCEPT("Syntax error on line", line_num, ". Expected a valid typename after 'as', like Integer or String or Long.");
					}

				}
			}
		}
		break;
	}
	default:
		return parse_state::ERR;
	}; // switch state
	
	return state;
}

int main()
{
	using std::string;
	pop_cmd_map(cmd_map);
	pop_type_map(type_map);

	string svb = vb();
	sjk::str::to_lower(svb);
	vec_t v;
	vec_t words;
	split("\n", svb, v);
	
	parse_state state(parse_state::NONE);
	int line = 0;
	for (auto& s : v)
	{
		line++;
		
		if (s.empty())
		{
			// ignore blank lines
			continue;
		}
		
		trim(s);
		split(" ",s, words);
		if (words.empty()) continue;
		state = parse(line, words, state);
		assert(0);

	}
	
	return 0;
}