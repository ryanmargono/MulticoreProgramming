#include <iostream>
#include "ThreadSafeKVStore.cpp"
#include "ThreadSafeListenerQueue.cpp"
#include <unordered_set>
#include <pthread.h>
#include <unistd.h>
#include "ThreadTime.cpp"
#include <random>
#include <getopt.h>
#include <set>

using namespace std;

// decalre functions and variables
int32_t get_value();
string get_key();
int get_probability();
int total_threads;
void *test(void* args);
ThreadSafeListenerQueue<int32_t> *ts_queue;
ThreadSafeKVStore<string, int32_t > *ts_map;
unordered_set<string> keys;
random_device rd;
mt19937 gen(rd());

int main(int argc, char* argv[]) {
    
    // get number of threads from command line
    if (getopt(argc, argv, "n:") != -1) {
        total_threads = atoi(optarg);
    }
    else {
        cout << "FATAL ERROR: arguments" << endl;
        pthread_exit(NULL);
    }

    // initialize data structures
    keys = {};
    ts_map = new ThreadSafeKVStore<string, int>;
    ts_queue = new ThreadSafeListenerQueue<int32_t>;
    pthread_t threads[total_threads];
    struct thread_times times[total_threads];
    
    // create threads and run tests
    for (int i = 0; i < total_threads; i++) {
        pthread_create(&threads[i], NULL, test, (void *)&times[i]);
    }

    // sum values from the queue
    int queue_total = 0;
    for (int i = 0; i < total_threads; i++) {
        int32_t local_sum = 0;
        ts_queue->listen(local_sum);
        queue_total += local_sum;
    }

    // wait for all threads to finish running tests
    for (int i=0; i< total_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // get sum from map
    int map_total = 0;
    for (unordered_map<string,int32_t >::iterator it=ts_map->data.begin(); it!=ts_map->data.end(); it++){
        map_total += it->second;
    }
        
    // print thread runtime data
    for (int i = 0; i < total_threads; i++) {
        cout << "thread " << i << " completion time: " << times[i].end - times[i].start << endl;
    }
    cout << "time between first launch and final terminating threads: " << times[total_threads-1].end - times[0].start << endl;

    // print final sums
    cout << "queue sum: " << queue_total << endl;
    cout << "map sum: " << map_total << endl;
    pthread_exit(NULL);
}

// thread tests
void *test(void* threadarg) {
    
    // start recording thread run times
    struct thread_times *thread_time;
    thread_time = (struct thread_times *) threadarg;
    thread_time->start = clock();

    // test logic
    int32_t total = 0;
    for (int i = 0; i < 10000; i++) {
        if (get_probability() < 20) {
            string key = get_key();
            int32_t value = get_value();
            ts_map->accumulate(key, value);
            keys.insert(key);
            total += value;
        } else {
            if (!keys.empty()) {
                auto it = keys.begin();
                string key = *it;     
                int32_t value = 0;
                if (ts_map->lookup(key, value) == false) {
                    cout << "Fatal Error: key does not exist." << endl;
                }
            }
        }
    }
    ts_queue->push(total);
    thread_time->end = clock();
    pthread_exit(NULL);
}

// test utility functions
int get_probability() {
    uniform_int_distribution<int32_t > dis(0, 100);
    return dis(gen);
}
string get_key() {
    uniform_int_distribution<> dis(0, 500);
    return "user" + to_string(dis(gen));;
}
int32_t get_value() {
    uniform_int_distribution<int32_t > dis(-256, 256);
    return dis(gen);
}

