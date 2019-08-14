#include <unistd.h>
#include "Maze.hpp"
#include <atomic>
#include <vector>
#include <thread>
#include "ThreadSafeKVStore.cpp"
#include "ThreadSafeListenerQueue.cpp"
#include <random>
#include <utility>

// declare globals
int threads;
int rows;
int columns;
int genome_size;
int max_futility;
int futility_counter = 0;
ThreadSafeKVStore<int, vector<int>> *population;
ThreadSafeListenerQueue<vector<int>> *offspring;
vector<thread> *mixer_threads;
vector<thread> *mutator_threads;
Maze *maze;
int get_fitness();

// computes fitness score for genome
int get_fitness(vector<int> genome)
{

    // iteration metrics
    int minimum_row = maze->getStart().row - 1;
    int maximum_row = maze->getFinish().row + 1;
    int result_row = minimum_row + 1;

    int minimum_column = maze->getStart().col - 1;
    int maximum_column = maze->getFinish().col + 1;
    int result_column = minimum_column + 1;

    int walls = 0;

    // fitness calculation logic (moves according to current allele)
    for (int i = 0; i < genome.size(); i++)
    {
        int gene = genome.at(i);
        switch (gene)
        {
        case 1:
            if (result_row > minimum_row)
                maze->get(result_row - 1, result_column) ? walls++ : result_row--;
        case 2:
            if (result_row < maximum_row)
                maze->get(result_row + 1, result_column) ? walls++ : result_row++;
        case 3:
            if (result_column > minimum_column)
                maze->get(result_row, result_column - 1) ? walls++ : result_column--;
        case 4:
            if (result_column < maximum_column)
                maze->get(result_row, result_column + 1) ? walls++ : result_column++;
        }
    }

    // return fitness
    return 2 * (maze->getFinish().row - result_row) + (maze->getFinish().col - result_column) + walls;
}

// mates two candidates from population, mixing their genomes
void mix()
{
    while (true)
    {

        random_device seed;
        default_random_engine gen(seed());

        // select two random candidates from population
        uniform_int_distribution<int> get_random_row(0, 4 * threads - 1);
        vector<int> candidate_a = population->access(get_random_row(gen)).second;
        vector<int> candidate_b = population->access(get_random_row(gen)).second;

        // candidate_a's genes will be added sequentially until a random split point, followed by candidate_b's.
        uniform_int_distribution<int> get_random_index(0, genome_size - 2);
        int a_stopping_point = get_random_index(gen);

        vector<int> result_genome;
        for (int i = 0; i <= a_stopping_point; i++)
        {
            result_genome.push_back(candidate_a.at(i));
        }
        for (int i = a_stopping_point + 1; i < genome_size; i++)
        {
            result_genome.push_back(candidate_b.at(i));
        }

        // add resulting genome to the offspring store
        offspring->push(result_genome);
    }
}

// alters the genomes
void mutate()
{
    while (true)
    {
        random_device seed;
        default_random_engine gen(seed());

        // save the gloabl best fitness for comparison
        int current_best_fitness = population->access(0).first;

        // get next offspring to mutate
        vector<int> new_genome;
        offspring->listen(new_genome);

        // has a 40% chance to mutate one random element in the offspring
        uniform_int_distribution<int> chance_to_mutate(1, 100);
        uniform_int_distribution<int> get_random_number(0, 4);
        uniform_int_distribution<int> get_random_index(0, genome_size - 1);
        if (chance_to_mutate(gen) <= 40)
        {
            new_genome.at(get_random_index(gen)) = get_random_number(gen);
        }

        // computes fitness of new genome and inserts it back into the population
        int fitness = get_fitness(new_genome);
        population->insert(fitness, new_genome);

        // truncate population
        population->truncate(4 * threads);

        // fetch first row's fitness and compares it to the new genome's. Adjust futility counter accordingly.
        int global_best_fitness = population->access(0).first;
        global_best_fitness < current_best_fitness ? futility_counter = 0 : futility_counter++;

        // program terminates once futility counter reaches maximum threshold and prints best result.
        if (futility_counter == max_futility)
        {
            cout << "Maximum futility reached - program terminating\n"
                 << endl;
            cout << "Maze:" << endl;
            cout << *maze << endl;
            cout << "Best Fitness: " << population->access(0).first << endl;
            cout << "Resulting Genome : ";

            vector<int> genome = population->access(0).second;
            for (int i = 0; i < genome.size(); i++)
            {
                cout << genome.at(i);
            }
            cout << endl;

            abort();
        }
    }
}

int main(int argc, char **argv)
{

    // read arguments from command line
    threads = atoi(argv[1]);
    rows = atoi(argv[2]);
    columns = atoi(argv[3]);
    genome_size = atoi(argv[4]);
    max_futility = atoi(argv[5]);

    int max = threads * 4;

    // initialize data structures
    population = new ThreadSafeKVStore<int, vector<int>>;
    offspring = new ThreadSafeListenerQueue<vector<int>>;
    maze = new Maze(rows, columns);
    mixer_threads = new vector<thread>;
    mutator_threads = new vector<thread>;

    // create initial random genome
    random_device seed;
    default_random_engine gen(seed());
    uniform_int_distribution<int> rand_dir(0, 4);

    for (int i = 0; i < max; i++)
    {
        vector<int> genome;
        for (int j = 0; j < genome_size; j++)
        {
            genome.push_back(rand_dir(gen));
        }
        population->insert(get_fitness(genome), genome);
    }

    // initialize threads
    for (int i = 0; i < threads / 2; i++)
    {
        mixer_threads->push_back(thread(mix));
        mutator_threads->push_back(thread(mutate));
    }

    for (int i = 0; i < threads / 2; i++)
    {
        mixer_threads->at(i).join();
        mutator_threads->at(i).join();
    }
}