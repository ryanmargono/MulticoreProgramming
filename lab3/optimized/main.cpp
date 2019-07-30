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
#include <cstdio>
#include <ctime>

using namespace std;

// decalre functions and variables
int N_Coefficients;
int N_Threads;
double Threshold;
vector<vector<int>> Coords;
ThreadSafeListenerQueue<Data *> *best_coefficients;
ThreadSafeListenerQueue<Data *> *best_results;
ThreadSafeListenerQueue<double> *thread_times;

void print(vector<double> const &input)
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

double evaluate(vector<double> coefficients, int x)
{
    double result = 0;
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

double get_distance(vector<int> coord, vector<double> coefficients)
{
    int x1 = coord[0];
    int y1 = coord[1];
    double y2 = evaluate(coefficients, x1);
    double result = (y1 - y2) * (y1 - y2);
    // cout << "distance: " << result << endl;
    return result;
}

double get_fitness(vector<double> coeffiecients)
{
    double result = 0;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        result += get_distance(Coords[i], coeffiecients);
    }
    return result;
}

double get_random_double(double min, double max)
{
    random_device rd;
    mt19937 eng(rd());
    uniform_real_distribution<> distribution(min, max);
    double result = distribution(eng);
    // cout << min << " " << max << " " << result << endl;
    return result;
}

vector<double> generate_coefficients()
{
    // cout << "generating random coefficients: " << endl;
    vector<double> coefficients;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        double random = get_random_double(-10, 10);
        coefficients.push_back(random);
        // cout << random << endl;
    }
    // cout << endl;
    return coefficients;
}

vector<double> mutate(vector<double> coefficients, double fitness)
{
    vector<double> result;
    double bounds = log (fitness); // adjust range of change based on fitness
    // cout << fitness << " " << bounds <<endl;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        double mutated = coefficients[i] + get_random_double(bounds * -1 , bounds);
        result.push_back(mutated);
    }
    return result;
}

void *run_alg(void *threadid)
{
    // record thread times
    clock_t thread_start;
    double thread_duration;
    thread_start = clock();

    // each thread will repeat this process until main terminates the program.
    while (true)
    {
        // get current best data from queue
        Data *global_best;
        best_coefficients->listen(global_best);

        // mute coefficients
        Data *local_best = new Data();
        local_best->coefficients = mutate(global_best->coefficients, global_best->fitness);
        local_best->fitness = get_fitness(local_best->coefficients);

        // push data to results for main to process
        best_results->push(local_best);

        // store thread time info
        thread_duration = (clock() - thread_start ) / (double) CLOCKS_PER_SEC;
        thread_times -> push(thread_duration);
    }
}

int main(int argc, char *argv[])
{

    // start timer for total runtime
    clock_t start;
    double duration;
    start = clock();

    // read arguments from command line
    N_Threads = atoi(argv[1]);
    N_Coefficients = atoi(argv[2]) + 1;
    Threshold = atoi(argv[3]) + 0.0;

    best_results = new ThreadSafeListenerQueue<Data *>;
    best_coefficients = new ThreadSafeListenerQueue<Data *>;
    thread_times = new ThreadSafeListenerQueue<double>;

    // generate random coords, random int between -5 and 5.
    cout << "\nrandomized coords:" << endl;
    int i;
    for (i = 0; i < N_Coefficients; i++)
    {
        // random_device rd;
        // mt19937 eng(rd());
        // uniform_int_distribution<> distr(-5, 5);
        // int x = distr(eng);
        // int y = distr(eng);
        // vector<int> current_coord = {x, y};
        // Coords.push_back(current_coord);
        // cout << "(" << x << "," << y << ")" << endl;

        if (i==0) {
            vector<int> current_coord = {-1, 3};
            Coords.push_back(current_coord);
        }
        if (i==1){
            vector<int> current_coord = {-4, 2};
            Coords.push_back(current_coord);
        }
        if (i==2){
            vector<int> current_coord = {-3, -2};
            Coords.push_back(current_coord);
        }
        if (i==3){
            vector<int> current_coord = {-5, 1};
            Coords.push_back(current_coord);
        }
        if (i==4){
            vector<int> current_coord = {4, 1};
            Coords.push_back(current_coord);
        }
    }
    cout << Coords.size() << endl;
    cout << endl;

    // generate first random coefficients and evaluate
    vector<double> coefficients = generate_coefficients();
    double fitness = get_fitness(coefficients);

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

    int total_guesses = 0;
    int best_guesses = 0;
    double best_fitness = best_data->fitness;

    // listen to the result queue. if the result is under a certain threshold, terminate program.
    while (best_fitness >= Threshold)
    {
        cout << best_fitness << endl;
        if (best_fitness == Threshold) cout << 'here' << endl;
        // listen to results queue
        Data *possible_best;
        best_results->listen(possible_best);
        total_guesses ++;

        // update best coefficients if found
        if (possible_best->fitness < best_data->fitness)
        {
            // cout << "best: " << possible_best->fitness << endl;
            best_guesses ++; 
            best_data->fitness = possible_best->fitness;
            best_data->coefficients = possible_best->coefficients;
            best_fitness = best_data->fitness;
        }
        // requeue best coefficients to be used if the threshold hasn't been met. Workers won't be given coefficients that are not progressive.
        best_coefficients->push(best_data);
    }

    // record thread stats
    double min;
    double max;
    double median;
    double mean;
    
    vector<double> thread_times_data;
    for (auto ci = thread_times->data.begin(); ci != thread_times->data.end(); ++ci) {
        thread_times_data.push_back((double)*ci);
    }
    sort(thread_times_data.begin(), thread_times_data.end());
    
    int size = thread_times_data.size();
    if (size % 2 == 0)
    {
    median = (thread_times_data[size / 2 - 1] + thread_times_data[size / 2]) / 2;
    }
    else 
    {
    median = thread_times_data[size / 2];
    }
    
    min = thread_times_data[0];
    max = thread_times_data.back();
    mean = accumulate(thread_times_data.begin(), thread_times_data.end(), 0.0)/size; 

    // print results
    print_result(best_data);
    
    cout <<"\nthread min time: " << min <<endl;
    cout <<"thread max time: " << max <<endl;
    cout <<"thread median time: " << median <<endl;
    cout <<"thread mean time: " << mean <<endl;

    duration = (clock() - start ) / (double) CLOCKS_PER_SEC;
    cout<<"\ntotal runtime: "<< duration << endl;
    cout<<"total thread iterations: "<< total_guesses << endl;
    cout<<"total best guesses: " << best_guesses << endl;
    exit(0);
};
