#pragma once

// An insertion ordered map, hacked together quickly for 
// usage with the profiler.
template <class TKey, class T>
class InsertionOrderedMap
{
public:
    InsertionOrderedMap() {}
    ~InsertionOrderedMap()
    {
        for (auto it : entries)
        {
            delete it;
        }
    }

    T *Find(const TKey key)
    {
        const auto searchResult = keyToIndexMap.find(key);
        if (searchResult != std::end(keyToIndexMap))
        {
            return entries[searchResult->second];
        }

        return nullptr;
    }

    T *Insert(const TKey key, const T &value)
    {
        T *entry = Find(key);
        if (!entry)
        {
            size_t index = entries.size();
            keyToIndexMap[key] = index;
            entries.push_back(new T(value));
            entry = entries[index];
        }

        return entry;
    }

    typename std::vector<T*>::iterator begin() { return entries.begin(); }
    const typename std::vector<T*>::const_iterator begin() const { return entries.begin(); }
    typename std::vector<T*>::iterator end() { return entries.end(); }
    const typename std::vector<T*>::const_iterator end() const { return entries.end(); }

private:
    std::unordered_map<TKey, size_t> keyToIndexMap;
    std::vector<T*> entries;
};