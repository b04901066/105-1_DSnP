/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;
   public:
      iterator(vector<HashNode>* b = NULL, size_t d = 0,
               vector<HashNode>* e = NULL): _bucket(b) , _idx(d) , _EOH(e) {}
      iterator(const iterator& i): _bucket(i._bucket) , _idx(i._idx) , _EOH(i._EOH) {}
      ~iterator() {}

      const HashNode& operator * () const { return (*_bucket)[_idx]; }
      HashNode& operator * () { return (*_bucket)[_idx]; }
      iterator& operator ++ (){ // Prefix
         ++_idx;
         if(_idx >= _bucket->size()){
            _idx = 0;
            do{ ++_bucket; }while( _bucket<_EOH && _bucket->empty());
         }
         return (*this);
      }
      iterator  operator ++ (int) { iterator temp(*this); ++(*this); return temp; } // Postfix
      iterator& operator -- (){ // Prefix
         if(_idx > 0){ --_idx;}
         else{
            do{ --_bucket; }while(_bucket->empty());
            _idx = _bucket->size()-1;
         }
         return (*this);
      }
      iterator  operator -- (int) { iterator temp(*this); --(*this); return temp; } // Postfix

      iterator& operator = (const iterator& i) {
         this->_bucket = i._bucket; this->_idx = i._idx; this->_EOH = i._EOH; return (*this); }
      bool operator == (const iterator& i) const {
         return ((this->_bucket == i._bucket)&&(this->_idx == i._idx)); }
      bool operator != (const iterator& i) const {
         return ((this->_bucket != i._bucket)||(this->_idx != i._idx)); }

   private:
      vector<HashNode>*  _bucket;
      size_t             _idx;
      vector<HashNode>*  _EOH;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = NULL; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const {
      for(size_t i = 0 ; i < _numBuckets ; ++i )
         if(!(_buckets+i)->empty())return iterator(_buckets+i,0,_buckets+_numBuckets);
      return end();
   }
   // Pass the end
   iterator end() const {
      return iterator(_buckets+_numBuckets,0,_buckets+_numBuckets); }
   // return true if no valid data
   bool empty() const { return (begin() == end()); }
   // number of valid data
   size_t size() const {
      size_t s = 0;
      for(size_t i = 0 ; i < _numBuckets; ++i )
         s += _buckets[i].size();
      return s;
   }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const {
      for(size_t i = 0 ; i < _buckets[bucketNum(k)].size() ; ++i )
         if(k == _buckets[bucketNum(k)][i].first)return true;
      return false;
   }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const {
      for(size_t i = 0 ; i < _buckets[bucketNum(k)].size() ; ++i )
         if(k == _buckets[bucketNum(k)][i].first){
            d =  _buckets[bucketNum(k)][i].second;
            return true;
         }
      return false;
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) {
      for(size_t i = 0 ; i < _buckets[bucketNum(k)].size() ; ++i )
         if(k == _buckets[bucketNum(k)][i].first){
            _buckets[bucketNum(k)][i].second = d;
            return true;
         }
      _buckets[bucketNum(k)].push_back(HashNode(k,d));
      return false;
   }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) {
      if(check(k))return false;
      _buckets[bucketNum(k)].push_back(HashNode(k,d));
      return true;
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) {
      for(size_t i = 0 ; i < _buckets[bucketNum(k)].size() ; ++i )
         if(k == _buckets[bucketNum(k)][i]){
            _buckets[bucketNum(k)].erase(_buckets[bucketNum(k)].begin()+i);
            return true;
         }
      return false;
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//
//    size_t operator() () const { return 0; }
//
//    bool operator == (const CacheKey&) const { return true; }
//
// private:
// };
//
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
