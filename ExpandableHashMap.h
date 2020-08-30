#ifndef EXPHASHMAP
#define EXPHASHMAP

#include <vector>
#include <list>
#include <string>
#include <functional>
#include <algorithm>
#include <iostream>

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5);
	~ExpandableHashMap();
	void reset();
	int size() const;
	void associate(const KeyType& key, const ValueType& value);

	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
	struct Node {
		KeyType key;
		ValueType val;
	};
	int numVals;
	int numBuckets;
	double maxLoad;
	std::list<Node>* hashMap;
};

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor) {
	if (maximumLoadFactor > 0) {
		maxLoad = maximumLoadFactor;
	}
	else {
		maxLoad = 0.5;
	}
	numVals = 0;
	numBuckets = 8;
	hashMap = new std::list<Node>[8];
}

template<typename KeyType, typename ValueType>
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap() {
	for (int i = 0; i < numBuckets; i++) { // loop through hash map and delete
		hashMap[i].erase(hashMap[i].begin(), hashMap[i].end());
	}
	delete[] hashMap;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::reset() {
	for (int i = 0; i < numBuckets; i++) { // delete the current hash map
		hashMap[i].erase(hashMap[i].begin(), hashMap[i].end());
	}
	delete[] hashMap;
	numBuckets = 8;
	numVals = 0;
	hashMap = new std::list<Node>[8]; // create a new one with 8 buckets and no associations
}

template<typename KeyType, typename ValueType>
int ExpandableHashMap<KeyType, ValueType>::size() const {
    return numVals;
}

template<typename KeyType, typename ValueType>
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value) {
	unsigned int hasher(const KeyType & key);
	unsigned int bucket = hasher(key) % numBuckets; // determine the correct bucket
	auto it = hashMap[bucket].begin();
	bool added = false;
	while (it != hashMap[bucket].end() && added == false) { // if it hasn't been added yet, keep going
		if ((*it).key == key) { // if the key is equal to one already in the bucket, update the value
			(*it).val = value;
			added = true;
		}
		it++;
	}
	if (!added) { // if it hasn't been added yet (ie the key didn't already exist), add it
		Node n;
		n.key = key;
		n.val = value;
		hashMap[bucket].push_back(n);
		numVals++;
	}

	double load = double(numVals) / double(numBuckets); // calculate load
	if (load > maxLoad) { // if the load exceeds the max load, a new map must be created
		std::list<Node>* oldHashMap = hashMap;
		hashMap = new std::list<Node>[numBuckets * 2]; // new hash map that's 2x as big
		for (int i = 0; i < numBuckets; i++) {
			auto it = oldHashMap[i].begin();
			while (it != oldHashMap[i].end()) { // loop through old one and rehash values into new map
				Node p = *it;
				unsigned int b = hasher(p.key) % (numBuckets * 2);
				hashMap[b].push_back(p);
				it++;
			}
			oldHashMap[i].erase(oldHashMap[i].begin(), oldHashMap[i].end()); // delete old hash map
		}
		delete[] oldHashMap;
		numBuckets = numBuckets * 2;
	}
}

template<typename KeyType, typename ValueType>
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const {
	unsigned int hasher(const KeyType& key);
	unsigned int bucket = hasher(key) % numBuckets; // determine the bucket where the key should be
	auto it = hashMap[bucket].begin();
	while (it != hashMap[bucket].end()) { // loop through that bucket
		if ((*it).key == key) { // if the key is found in the node, return the value
			ValueType* v = &((*it).val);
			return v;
		}
		it++;
	}
	return nullptr;
}

#endif