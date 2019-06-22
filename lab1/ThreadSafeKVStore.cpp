#include <unordered_map>
#include <mutex>

using namespace std;

template <class K, class V> class ThreadSafeKVStore {
private:
    mutex mut;
public:
    unordered_map<K,V> data;
    ThreadSafeKVStore() {
        data = unordered_map<K,V>();
    }
    bool insert(K key, V value);
    bool accumulate(const K key, const V value);
    bool lookup(const K key, V& value);
    bool remove(const K key);
};

template <class K,class V> bool ThreadSafeKVStore<K,V>::insert(K key, V value) {
    lock_guard<mutex> lk(mut);
    data[key] = value;
    return true;
}

template <class K,class V> bool ThreadSafeKVStore<K,V>::accumulate(const K key, const V value) {
    lock_guard<mutex> lk(mut);
    auto search = data.find(key);
    if (search != data.end()) {
        data[key] = search->second + value;
    } else {
        data[key] = value;
    }
    return true;
}

template <class K,class V> bool ThreadSafeKVStore<K,V>::lookup(const K key, V &value) {
    lock_guard<mutex> lk(mut);
    auto search = data.find(key);
    if(search != data.end()) {
        value = search->second;
        return true;
    }
    return false;
}

template <class K,class V> bool ThreadSafeKVStore<K,V>::remove(const K key) {
    lock_guard<mutex> lk(mut);
    auto search = data.find(key);
    if(search != data.end()) {
        data.erase(key);
    }
    return true;
}