#include <map>
#include <utility>
#include <shared_mutex>
#include <mutex>

using namespace std;

template <typename K, typename V>
class ThreadSafeKVStore
{
private:
    shared_mutex reader_writer_lock; // reader-writer lock
    multimap<K, V> data;

public:
    pair<K, V> access(const int index); //  chose to use pairs because its the easiest heterogenous data container for this use case.
    bool insert(const K key, const V val);
    bool truncate(const int size);
};

// allows 'random access' in the multimap
template <typename K, typename V>
pair<K, V> ThreadSafeKVStore<K, V>::access(const int index)
{
    reader_writer_lock.lock_shared();
    typename multimap<K, V>::iterator it;

    // trying to access an index that is bigger than the size of the multimap returns the last element instead.
    if (index >= data.size())
    {
        it = data.end() - 1;
        reader_writer_lock.unlock_shared();
        return *it;
    }

    // returns the element in specified index
    it = data.begin();
    for (int i = 0; i < index; i++)
    {
        it++;
    }
    reader_writer_lock.unlock_shared();
    return *it;
}

template <typename K, typename V>
bool ThreadSafeKVStore<K, V>::insert(const K key, const V val)
{
    reader_writer_lock.lock();
    
    // creates a std::pair object and inserts it to the multimap
    data.insert(make_pair(key, val));
    reader_writer_lock.unlock();
    return true;
}

template <typename K, typename V>
bool ThreadSafeKVStore<K, V>::truncate(const int size)
{
    reader_writer_lock.lock();

    // shortens multimap by input number
    typename multimap<K, V>::iterator it = data.begin();
    for (int i = 0; i < size; i++)
    {
        it++;
    }
    data.erase(it, data.end());
    reader_writer_lock.unlock();
    return true;
}