#pragma once
#include <string>
#include <time.h>
#include <iostream>
class BenchMark {
public:
	BenchMark(std::string str) {
		name = str;
		time(&begin);
	}
	BenchMark() = delete;
	~BenchMark() {
		time(&end);
		std::cout << "[" << name << "] cost " << end - begin << " s" << std::endl;
	}
private:
	time_t begin, end;
	std::string name;
};
