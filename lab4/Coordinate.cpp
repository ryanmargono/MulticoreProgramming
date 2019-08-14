#include <algorithm>
#include <cmath>
#include <iostream>

struct Coord {
	Coord(void)
		: row(0), col(0)
	{}
	Coord(const size_t row0, const size_t col0)
		: row(row0), col(col0)
	{}
	size_t row;
	size_t col;
};
