#include <atomic>
#include <vector>
#include <thread>
#include "ThreadSafeKVStore.cpp"
#include "ThreadSafeListenerQueue.cpp"
#include "Maze.hpp"

using namespace std;

class MazeSolver {
    ThreadSafeKVStore<int, vector<int> > population_;
    ThreadSafeListenerQueue<vector<int> > offspring_;
    atomic<unsigned long long> counter_;
    vector<thread> mixers_;
    vector<thread> mutaters_;
    int rows_;
    int cols_;
    int threads_;
    int threshold_;
    int genome_size_;
    public:
    MazeSolver(int rows, int cols, int genome_size, int threshold, int threads);
    void run();
    void mix(Maze maze);
    void mutate(Maze maze);
    int getFitness(Maze maze, vector<int> genome);
};

