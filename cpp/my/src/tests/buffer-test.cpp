// buffer-test.cpp

#include "../../include/my.h"
#include "../../include/my_array.h"

using my::cerr;
using my::cout;
using my::endl;

void test_overlaps() {

	int x = 0;
	int y = 0;
	using range = my::memory::range<int>;
	
	range rx(x);
	range rx2(x);

	range ry(y);
	range ry2(y);

	int ol = memory_overlaps(rx, rx);
	ASSERT(ol);

	ol =  memory_overlaps(rx, rx2);
	ASSERT(ol);

	ol = memory_overlaps(rx, ry);
	ASSERT(!ol);

	ol = memory_overlaps(rx2, ry2);
	ASSERT(!ol);

	int* px = new int;
	int* py = new int;
	*px = 77;
	*py = 77;

	range rpx(*px);
	range rpy(*py);

	ol = memory_overlaps(rpx, rpx);
	ASSERT(ol);

	ol = memory_overlaps(rpy, rpx);
	ASSERT(!ol);

	delete px;
	delete py;

	using crange = my::memory::range<const char>;
	char buf[1024];

	crange br1(buf, buf + 9);
	crange br2(buf, buf + 1024);
	crange br3(buf + 10, buf + 20);
	crange br4(nullptr, nullptr);

	ol = memory_overlaps(br1, br1);
	ASSERT(ol);

	ol = memory_overlaps(br1, br2);
	ASSERT(ol);

	ol = memory_overlaps(br1, br3);
	ASSERT(!ol);

	ol = memory_overlaps(br1, br4);
	ASSERT(!ol);

	ol = memory_overlaps(br4, br4);
	ASSERT(ol);

	


	
}

int main()
{
	{
		test_overlaps();
		
		my::memory::array<int> a;
		ASSERT(a.empty());
		a.push_back(1);
		ASSERT(a.size() == 1);
		a.push_back(2);
		ASSERT(a.size() == 2);
		a.push_back(3);
		ASSERT(a.size() == 3);
		a.clear();
		ASSERT(a.empty());
		ASSERT(a.size() == 0);

	}

	cout << "All done, for now!" << endl;
	return 0;
}
