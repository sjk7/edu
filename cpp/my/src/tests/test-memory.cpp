// test-memory.cpp

#include "../../include/my.h"
#include "../../include/my_type_safe.h"

using my::cout;
using my::cerr;
using my::endl;

struct x {};
struct y {};

typedef my::number_type<x, int> num_x;
typedef my::number_type<y, int> num_y;

void func(num_x x, num_y y){}
void func2(num_x& x, num_y& y) {}
void func3(const num_x& x, const num_y& y) {}


int main() {
	num_x x(100);
	num_y y (200);
	int a = 0;

	func(x, y);

	//func(a, y);
	func2(x, y);
	cout << x << endl;
	++x;
	cout << x << endl;
	x++;
	cout << x << endl;
	x = x * 10;
	cout << x << endl;




}

/*/
my::number<int> funca(int x, my::number<int> y)
{

	return my::number<int>(y);
}

void funcb(my::number<uint8_t> a, my::number<int> b)
{
	return;
}
int main()
{
	int c = 0;
	// func(c, c);

	my::number<int> a(42);
	my::number<int> b(64);

	// uint8_t x = (uint8_t)a;
	cout << "a = " << a << endl;
	cout << "b = " << (int)b << endl;
	b = a;
	cout << "a = " << (int)a << endl;
	cout << "b = " << (int)b << endl;

	
	//int z = a; 
	//func(a, b);
	//func(a, c);
	
	a += 10;
	assert(a == 52);
	a -= 10;
	assert(a == 42);
	a = funca(c, a);

	my::number<uint8_t>ui8(0);
	my::number<int>i32(0);
	ui8 += 1;
	assert(ui8 == 1);
	ui8++;
	assert(ui8 == 2);
	++ui8;
	assert(ui8 == 3);
	funcb(ui8, i32);
	//funcb(1, 10);

	ui8 += 10;
	assert(ui8 == 13);
	my::number<uint8_t>n(10);
	ui8 += n;
	assert(ui8 == 23);
	assert(a == 42);
	cout << "done." << endl;
}
/*/