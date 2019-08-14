#include <random>
#include "solver.hpp"

using namespace std;

MazeSolver::MazeSolver(int rows, int cols, int genome_size, int threshold, int threads) {
    rows_ = rows;
    cols_ = cols;
    genome_size_ = genome_size;
    threshold_ = threshold;
    threads_ = threads;
    counter_ = 0;
}

int MazeSolver::getFitness(Maze maze, vector<int> genome) {
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

void MazeSolver::mix(Maze maze) {
    while (true) {
        random_device seed;
        default_random_engine gen(seed());
        uniform_int_distribution<int> rand_row(0, 4 * threads_ - 1);
        uniform_int_distribution<int> rand_idx(0, genome_size_ - 2);
        vector<int> genome_a = population_[rand_row(gen)].second;
        vector<int> genome_b = population_[rand_row(gen)].second;
        vector<int> genome_child;
        int splice_point = rand_idx(gen);
        for (int i = 0; i <= splice_point; i++) {
            genome_child.push_back(genome_a.at(i));
        }
        for (int i = splice_point + 1; i < genome_size_; i++) {
            genome_child.push_back(genome_b.at(i));
        }
        offspring_.push(genome_child);
    }
}

void MazeSolver::mutate(Maze maze) {
    while (true) {
        random_device seed;
        default_random_engine gen(seed());
        uniform_int_distribution<int> prob_num(1, 100);
        uniform_int_distribution<int> rand_num(0, 4);
        uniform_int_distribution<int> rand_idx(0, genome_size_ - 1);
        int best_fit = population_[0].first;
        vector<int> genome_mix;
        offspring_.listen(genome_mix);

        if (prob_num(gen) <= 40) {
            genome_mix.at(rand_idx(gen)) = rand_num(gen);
        }

        int fitness = getFitness(maze, genome_mix);
        population_.insert(fitness, genome_mix);
        population_.truncate(4 * threads_);
        int new_best_fit = population_[0].first;
        if (new_best_fit < best_fit) {
            counter_ = 0;
        } else {
            counter_++;
        }

        if (counter_ == threshold_) {
            int best_fitness = population_[0].first;
            vector<int> genome = population_[0].second;
            cout << maze << endl;
            cout << "Genome: ";
            for (int i = 0; i < genome.size(); i++) {
                cout << genome.at(i);
            }
            cout << endl;
            cout << "Fitness: " << best_fitness << endl;
            abort();
        }
    }
}

void MazeSolver::run() {
    srand(time(0));
    Maze maze(rows_, cols_);
    random_device seed;
    default_random_engine gen(seed());
    uniform_int_distribution<int> rand_dir(0, 4);
    
    for (int i = 0; i < 4 * threads_; i++) {
        vector<int> genome;
        for (int j = 0; j < genome_size_; j++) {
            genome.push_back(rand_dir(gen));
        }

        population_.insert(getFitness(maze, genome), genome);
    }

    for (int i = 0; i < threads_ / 2; i++) {
        mixers_.push_back(thread(&MazeSolver::mix, this, maze));
        mutaters_.push_back(thread(&MazeSolver::mutate, this, maze));
    }

    for (int i = 0; i < threads_ / 2; i++) {
        mixers_.at(i).join();  
        mutaters_.at(i).join();
    }
}


