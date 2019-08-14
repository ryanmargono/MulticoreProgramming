#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include "Coordinate.cpp"

using namespace std;

class Maze {

	void generate(void);
	void set(const size_t row, const size_t col, const bool val = true);
	void reset(const size_t row, const size_t col);

	void initialize(void);
	void carve(const size_t x, const size_t y);

	const size_t rows_, cols_;
	vector<bool> maze_;
	Coord start_, finish_;
	
  protected:
	ostream& show(ostream &os) const;
	friend ostream &operator<<(ostream &os, const Maze &maze);

  public:
	Maze(const size_t rows, const size_t cols);
	const bool get(const size_t row, const size_t col) const;
	const Coord getStart();
	const Coord getFinish();
};
