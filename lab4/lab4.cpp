#include <unistd.h>
#include "Maze.hpp"
#include <atomic>
#include <vector>
#include <thread>
#include "ThreadSafeKVStore.cpp"
#include "ThreadSafeListenerQueue.cpp"
#include <random>

int getFitness(Maze maze, vector<int> genome) {
    int min_row = maze.getStart().row - 1;
    int min_col = maze.getStart().col - 1;
    int max_row = maze.getFinish().row + 1;
    int max_col = maze.getFinish().col + 1;
    int curr_row = min_row + 1;
    int curr_col = min_col + 1;
    int wall_count = 0;

    for (int i = 0; i < genome.size(); i++) {
        if (genome.at(i) == 1) {
            if (curr_row > min_row) {
                if (maze.get(curr_row - 1, curr_col)) {
                    wall_count++;
                } else {
                    curr_row--;
                }
            }
        } else if (genome.at(i) == 2) {
            if (curr_row < max_row) {
                if (maze.get(curr_row + 1, curr_col)) {
                    wall_count++;
                } else {
                    curr_row++;
                }
            }
        } else if (genome.at(i) == 3) {
            if (curr_col > min_col) {
                if (maze.get(curr_row, curr_col - 1)) {
                    wall_count++;
                } else {
                    curr_col--;
                }
            }
        } else if (genome.at(i) == 4) {
            if (curr_col < max_col) {
                if (maze.get(curr_row, curr_col + 1)) {
                    wall_count++;
                } else {
                    curr_col++;
                }
            }
        }
    }
    int distance = (maze.getFinish().row - curr_row) + (maze.getFinish().col - curr_col);
    return 2 * distance + wall_count;
}

int main(int argc, char** argv) {

    int threads = atoi(argv[1]);
    int rows = atoi(argv[2]);
    int columns = atoi(argv[3]);
    int genome_size = atoi(argv[4]);
    int threshold = atoi(argv[5]);

    int max = threads * 4;
    
    ThreadSafeKVStore<int, vector<int> > population;
    ThreadSafeListenerQueue<vector<int> > offspring;
    Maze maze(rows, columns);

    random_device seed;
    default_random_engine gen(seed());
    uniform_int_distribution<int> rand_dir(0, 4);
    
    for (int i = 0; i < max; i++) {
        vector<int> genome;
        for (int j = 0; j < genome_size; j++) {
            genome.push_back(rand_dir(gen));
        }
        population.insert(getFitness(maze, genome), genome);
    }

    // for (int i = 0; i < threads / 2; i++) {
    //     mixers_.push_back(thread(&MazeSolver::mix, this, maze));
    //     mutaters_.push_back(thread(&MazeSolver::mutate, this, maze));
    // }

    // for (int i = 0; i < threads / 2; i++) {
    //     mixers_.at(i).join();  
    //     mutaters_.at(i).join();
    // }
}