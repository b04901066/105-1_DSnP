/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   const Data& min() const{ return _data.front(); }
   void insert(const Data& d){  // ReheapUp
      size_t insert_pos = _data.size();//new size-1
      _data.push_back(d);  //  ++size
      size_t parent_pos = (insert_pos-1)/2;
      while( insert_pos > 0 && d < _data[parent_pos]){
         _data[insert_pos] = _data[parent_pos];
         insert_pos = parent_pos;
         parent_pos = (insert_pos-1)/2;
      }
      _data[insert_pos] = d;
      return;
   }
   void delMin(){ this->delData(0); return; }
   void delData(size_t i){
      // ReheapUp
      size_t next = (i-1)/2;
      while( i > 0 && _data.back() < _data[next]){
         _data[i] = _data[next];
         i = next;
         next = (i-1)/2;
      }
      // ReheapDown
      next = i*2+1;
      size_t n = _data.size()-1;//new size
      while( next <  n ){
         if( next < (n-1) ) // has right child
            if(_data[next+1] < _data[next])
               ++next;   // right child is smaller
         if( _data.back() < _data[next] ) break;
         _data[i] = _data[next];
         i = next;
         next = i*2+1;
      }
      _data[i] = _data.back();
      _data.pop_back();
      return;
   }

private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H

