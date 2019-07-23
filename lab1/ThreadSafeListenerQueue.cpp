#include <list>
#include <mutex>
#include <condition_variable>

using namespace std;

template <class T> class ThreadSafeListenerQueue {
private:
    mutex mut;
    condition_variable data_cond;

public:
    list<T> data;
    ThreadSafeListenerQueue() {
        data = list<T>();
    }
    bool push(const T element);
    bool pop(T& element);
    bool listen(T& element);
    bool empty() const;
};

template <class T> bool ThreadSafeListenerQueue<T>::push(const T element) {
    // lock_guard<mutex> lk(mut);
    // data.push_back(element);
    // data_cond.notify_one();
    // return true;
}

template <class T> bool ThreadSafeListenerQueue<T>::pop(T &element) {
    lock_guard<mutex> lk(mut);
    if (data.empty()) {
        return false;
    }
    element = data.front();
    data.pop_front();
    return true;
}

template <class T> bool ThreadSafeListenerQueue<T>::listen(T &element) {
    unique_lock<mutex> lk(mut);
    data_cond.wait(lk, [this]{return !data.empty();});
    element = data.front();
    data.pop_front();
    return 0;
}

template <class T> bool ThreadSafeListenerQueue<T>::empty() const {
    lock_guard<mutex> lk(mut);
    return data.empty();
}