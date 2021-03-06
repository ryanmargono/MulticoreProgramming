#include <iostream>
#include "ThreadSafeListenerQueue.cpp"
#include <pthread.h>
#include <unistd.h>
#include "Data.cpp"
#include <random>
#include <cmath>

#include <vector>
#include <array>
#include <math.h>
#include <limits>
#include <random>

using namespace std;

// decalre functions and variables
int N_Coefficients;
int N_Threads;
float Threshold;
vector<vector<int>> Coords;
ThreadSafeListenerQueue<Data *> *best_coefficients;
ThreadSafeListenerQueue<Data *> *best_results;

void print(vector<float> const &input)
{
    for (int i = 0; i < input.size(); i++)
    {
        cout << input.at(i) << ' ';
    }
}

void print_result(Data *data)
{
    cout << "result: \ncoefficients: ";
    print(data->coefficients);
    cout << "\nfitness: " << data->fitness << endl;
}

float evaluate(vector<float> coefficients, int x)
{
    float result = 0;
    double degree = 1.0 * (N_Coefficients - 1);
    int i = 0;
    while (degree > 0)
    {
        result += coefficients[i] * pow(x * 1.0, degree);
        degree--;
        i++;
    }
    result += coefficients[i];
    // cout << "evaluation when x = " << x << ": y = " << result << endl;
    return result;
}

float get_distance(vector<int> coord, vector<float> coefficients)
{
    int x1 = coord[0];
    int y1 = coord[1];
    float y2 = evaluate(coefficients, x1);
    float result = (y1 - y2) * (y1 - y2);
    // cout << "distance: " << result << endl;
    return result;
}

float get_fitness(vector<float> coeffiecients)
{
    // cout << "processing coefficients: ";
    // print(coeffiecients);
    // cout << endl;

    float result = 0;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        result += get_distance(Coords[i], coeffiecients);
    }
    // cout << "fitness: " << result << "\n"<< endl;
    return result;
}

float get_random_float(float min, float max)
{
    random_device rd;
    mt19937 eng(rd());
    uniform_real_distribution<> distribution(min, max);
    float result = distribution(eng);
    // cout << min << " " << max << " " << result << endl;
    return result;
}

vector<float> generate_coefficients()
{
    // cout << "generating random coefficients: " << endl;
    vector<float> coefficients;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        float random = get_random_float(-10, 10);
        coefficients.push_back(random);
        // cout << random << endl;
    }
    // cout << endl;
    return coefficients;
}

vector<float> mutate(vector<float> coefficients, float fitness)
{
    vector<float> result;
    float bounds = log (fitness); // adjust range of change based on fitness
    // cout << fitness << bounds <<endl;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        float mutated = coefficients[i] + get_random_float(bounds * -1 , bounds);
        result.push_back(mutated);
    }
    return result;
}

void *run_alg(void *threadid)
{
    // each thread will repeat this process until main terminates the program.
    while (true)
    {
        // get current best data from queue
        Data *global_best;
        best_coefficients->listen(global_best);

        // mutate coords until we have coefficients that yield better fitness
        Data *local_best = new Data();
        local_best->fitness = global_best->fitness;
        local_best->coefficients = global_best->coefficients;
        float offset = 0;
        while (global_best->fitness <= local_best->fitness)
        {
            vector<float> temp_coefficients = mutate(local_best->coefficients, local_best->fitness);
            float temp_fitness = get_fitness(temp_coefficients);
            if (temp_fitness < local_best->fitness) 
            {
                local_best->coefficients = temp_coefficients;
                local_best->fitness = temp_fitness;
            }
        }

        // push data to results for main to process
        best_results->push(local_best);
    }
}

int main(int argc, char *argv[])
{

    // read arguments from command line
    N_Threads = atoi(argv[1]);
    N_Coefficients = atoi(argv[2]) + 1;
    Threshold = atoi(argv[3]);

    best_results = new ThreadSafeListenerQueue<Data *>;
    best_coefficients = new ThreadSafeListenerQueue<Data *>;

    // generate random coords, random int between -5 and 5.
    cout << "\nrandomized coords:" << endl;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        random_device rd;
        mt19937 eng(rd());
        uniform_int_distribution<> distr(-5, 5);
        int x = distr(eng);
        int y = distr(eng);
        vector<int> current_coord = {x, y};
        Coords.push_back(current_coord);
        cout << "(" << x << "," << y << ")" << endl;
    }
    cout << endl;

    // generate first random coefficients and evaluate
    vector<float> coefficients = generate_coefficients();
    float fitness = get_fitness(coefficients);

    Data *best_data = new Data();
    best_data->coefficients = coefficients;
    best_data->fitness = fitness;

    // save initial coords and fitness to be used as local best
    best_results->push(best_data);

    // push coefficients to queue for each thread
    for (i = 0; i < N_Threads; i++)
    {
        best_coefficients->push(best_data);
    }

    // create threads and run tests
    pthread_t threads[N_Threads];
    for (int i = 0; i < N_Threads; i++)
    {
        pthread_create(&threads[i], NULL, run_alg, NULL);
    }

    // listen to the result queue. if the result is under a certain threshold, terminate program.
    while (best_data->fitness > Threshold)
    {
        // listen to results queue
        Data *possible_best;
        best_results->listen(possible_best);

        // update best coefficients if found
        if (possible_best->fitness < best_data->fitness)
        {
            cout << "best: " << possible_best->fitness << endl;
            best_data->fitness = possible_best->fitness;
            best_data->coefficients = possible_best->coefficients;
        }
        // requeue best coefficients to be used if the threshold hasn't been met. Workers won't be given coefficients that are not progressive.
        best_coefficients->push(best_data);
    }
    print_result(best_data);
    exit(0);
};
