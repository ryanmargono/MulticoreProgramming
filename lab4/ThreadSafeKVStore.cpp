#include <mutex>
#include <map>
#include <utility>
#include <shared_mutex>

using namespace std;

template <typename K, typename V>
class ThreadSafeKVStore
{
    shared_mutex rwlock_;
    multimap<K, V> map_;

public:
    bool insert(const K key, const V val);
    pair<K, V> operator[](const int n);
    bool truncate(const int n);
};

template <typename K, typename V>
    bool ThreadSafeKVStore<K, V>::insert(const K key, const V val)
    {
        rwlock_.lock();
        map_.insert(make_pair(key, val));
        rwlock_.unlock();
        return true;
    }

template <typename K, typename V>
    pair<K, V> ThreadSafeKVStore<K, V>::operator[](const int n)
    {
        rwlock_.lock_shared();
        typename multimap<K, V>::iterator it;
        if (n >= map_.size())
        {
            it = map_.end();
            it--;
            rwlock_.unlock_shared();
            return *it;
        }
        it = map_.begin();
        for (int i = 0; i < n; i++)
        {
            it++;
        }
        rwlock_.unlock_shared();
        return *it;
    }

template <typename K, typename V>
    bool ThreadSafeKVStore<K, V>::truncate(const int n)
    {
        rwlock_.lock();
        typename multimap<K, V>::iterator it = map_.begin();
        for (int i = 0; i < n; i++)
        {
            it++;
        }
        map_.erase(it, map_.end());
        rwlock_.unlock();
        return true;
    }