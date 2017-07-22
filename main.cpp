#include <iostream>
#include <thread>
#include <chrono>
using std::cout;
using std::endl;
#include <vector>
using std::vector;
#include <algorithm>
#include <random>
#include <Fl/Fl.H>
#include <Fl/Fl_Window.H>
#include <Fl/Fl_Box.H>
#include <Fl/Fl_Widget.H>
#include <FL/Enumerations.H>

#define VISUALIZE

const static int sz = 32;

int index(int x, int y) {
	return x * sz + y;
}

struct user_input {
	bool clicked;
	int button;
	int x_index;
	int y_index;
} user_input;

class patch : public Fl_Box {
public:

	patch(int x, int y, int w, int h, int _x_index, int _y_index) : Fl_Box(x,y,w,h) {
		box(FL_UP_BOX);
		x_index = _x_index;
		y_index = _y_index;
	}

	void setBoxDown() {
		box(FL_FLAT_BOX);
	}
	void setLabel(const char* letter) {
		copy_label(letter);
	}

	int handle(int event) {
		int ret = Fl_Box::handle(event);
		switch (event) {
			case FL_PUSH:
				ret = 1;
				user_input.clicked = false;
				break;
			case FL_LEAVE:
				ret = 1;
				user_input.clicked = false;
				break;
			case FL_RELEASE:
				ret = 1;
				if (Fl::event_is_click()) {
					user_input.clicked = true;
					user_input.button = Fl::event_button()/3; // button/3 maps {1,3} to {0,1}
					user_input.x_index = x_index;
					user_input.y_index = y_index;
				} else {
					user_input.clicked = false;
				}
				break;
		}
		return(ret);
	}
private:
	int x_index;
	int y_index;
};

void print_vector(vector<int>& a, int sqr) {
	cout << endl;
	for (int i = 0; i < sqr; ++i) {
		for (int j = 0; j < sqr; ++j) {
			cout << a[i*sqr + j] << " ";
		}
		cout << endl;
	}
}

void clickSquare(int indx, vector<int>& board, vector<int>& covered, vector<patch*>& grid, Fl_Window* w) {
	covered[indx] = 0;
	grid[indx]->setBoxDown();
	if (board[indx] > 0)
		grid[indx]->setLabel(std::to_string(board[indx]).c_str());
#ifdef VISUALIZE
	w->redraw();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	Fl::wait();
#endif
}

void uncover(int x, int y, vector<int>& board, vector<int>& covered, vector<patch*>& grid, Fl_Window* w) {
	size_t indx = index(x,y);

	// Reveal number of surrounding mines to user
	clickSquare(indx, board, covered, grid, w);

	// If we clicked a square with no surrounding mines, then
	if (board[indx] == 0) {
		// uncover each surrounding square that is not a mine
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				int X = x;
				int Y = y;
				if (x+i >= 0 && x+i < sz)
					X=x+i;
				if (y+j >= 0 && y+j < sz)
					Y=y+j;
				indx = index(X,Y);
				if (covered[indx] && board[indx] >= 0)
					uncover(X,Y, board, covered, grid, w);
			}
		}
	}
}

// Convolution solution
void initialize_board(vector<int>& board) {
	vector<int> backup = board;
	vector<int> kernel(9, 1);
	kernel[4] = -9;

	for (int i = 0; i < sz; ++i) {
		for (int j = 0; j < sz; ++j) {
			int sum = 0;
			int indx = index(i,j);
			for (int ii = -1; ii <= 1; ++ii) {
				for (int jj = -1; jj <= 1; ++jj) {
					if (ii+i < 0 || jj+j < 0 || ii+i >= sz || jj+j >=sz)
						continue;
					sum += kernel[(ii+1)*3+(jj+1)] * backup[index(i+ii,j+jj)];
				}
			}
			board[indx] = sum;
		}
	}

	for (int& n : board)
		n = std::max(-1, n);
	print_vector(board, sz);
}

// Recursive solution
// todo

int main() {
	auto eng = std::default_random_engine(12345);
	auto dist = std::normal_distribution<float>();
	auto random_binary = [&]() {
		return fabs(dist(eng)) < .1 ? 1 : 0;
	};

	vector<int> board(sz*sz, 0);
	vector<int> covered(sz*sz, 1);
	vector<int> mark(sz*sz, 0);

	for (int& n : board)
		n = random_binary();

	initialize_board(board);

	// board array
	//    -1  mine
	//    0-7 number surrounding mines
	// covered array
	//    0 uncovered
	//    1 covered
	// mark array
	//    0 nothing
	//    1 flag
	//    2 question mark

	// generate board
	// input
	//    0 = no mine
	//    1 = mine
	// output
	//    -1  mine
	//    0-7 number surrounding mines

	// handle tap
	// input
	//    0 covered
	//    1 uncovered
	//    tapped patch (x,y)
	// output
	//    set tapped patch to 1
	//    if was board(x,y) == 0 then uncover all adjacent zeros eck

	// button
	//    0 left click
	//    1 right click

	Fl_Window* w = new Fl_Window(24*sz, 24*sz);
	vector<patch*> grid(sz*sz);
	for (int i = 0; i < sz; ++i)
		for (int j = 0; j < sz; ++j)
			grid[index(i,j)] = new patch(j*24, i*24, 24, 24, i, j);
	w->end();
	w->show();
	while (Fl::wait()) {
		if (user_input.clicked) {
			int x = user_input.x_index;
			int y = user_input.y_index;
			int indx = index(x,y);
			int button = user_input.button;
			if (covered[indx]) {
				if (!button) {
					if (mark[indx] == 0) {
						if (board[indx] == -1)
							goto GAMEOVER;
						uncover(x, y, board, covered, grid, w);
						w->redraw();
					}
				} else {

					mark[indx] = (mark[indx] + 1) % 3;
					switch (mark[indx]) {
					case 0:
						grid[indx]->setLabel(" \0");
						break;
					case 1:
						grid[indx]->setLabel("!\0");
						break;
					case 2:
						grid[indx]->setLabel("?\0");
						break;
					}
					w->redraw();

				}
			}
			// check end game conditions
			user_input.clicked = false;
		}
	}
GAMEOVER:
	cout << endl << "Game over" << endl;
	for (int i = 0; i < sz; ++i) {
		for (int j = 0; j < sz; ++j) {
			int indx = index(i,j);
			if (board[indx] == -1) {
				grid[indx]->setLabel(" \0");
				grid[indx]->color(FL_BLACK);
			}
		}
	}
	w->redraw();
	while (Fl::wait());
}
