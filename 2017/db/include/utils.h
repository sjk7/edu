#ifndef UTILS_H
#define UTILS_H


#include <fstream>
#include <chrono>

namespace cpp{
	inline auto filesize(std::fstream& f){
		auto g = f.tellg();
		auto p = f.tellp();

		f.seekg(0);
		f.seekg(0, std::ios_base::end);

		auto ret = f.tellg();

		f.seekg(g);
		f.seekp(p);
		return ret;
	}

	inline auto timer(){
		return
			std::chrono::system_clock::now().time_since_epoch() /
			std::chrono::milliseconds(1);
	}
}

#endif // UTILS_H
