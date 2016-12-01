#include <windows.h>
#include <cassert>
#include "../../../../include/sjk/sjk_enum_flags.h"
#include "../../../../include/sjk/sjk_terminal.h"
using namespace std;



enum class colors : unsigned int { r = 1, g = 2, b = 4 };
enum class shade : unsigned int {light, dark};
struct non_enum {};

int main()
{
	using namespace sjk::enum_flags;
	// Test the bitwise operators operator&, operator|, operator^, operator~, operator&=, operator|=, and operator^=

	// combine:
	colors c = colors::g | colors::r;
	assert((c & colors::g));
	assert(c & colors::r);
	assert(!(c & colors::b));
	
	// combine another:
	c |= colors::b;
	assert(c & colors::b);
	assert((c & colors::g));
	assert(c & colors::r);
	
	// toggle (1)
	c ^= colors::r;
	assert(!(c & colors::r));
	assert(c & colors::b);
	assert((c & colors::g));
	
	// toggle (again)
	c ^= colors::r;
	assert(c & colors::r);
	assert(c & colors::b);
	assert((c & colors::g));

	// switch off
	c &= ~(colors::g);
	assert(!(c & colors::g));
	assert(c & colors::r);
	assert(c & colors::b);

	return sjk::terminal::done();

}