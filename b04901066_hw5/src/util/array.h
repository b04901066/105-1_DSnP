/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0), _isSorted(true) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return (*this); }
      T& operator * () { return (*_node); }
      iterator& operator ++ ()    { ++_node; return (*this); }           // Prefix
      iterator  operator ++ (int) { ++_node; return iterator(_node-1); } // Postfix
      iterator& operator -- ()    { --_node; return (*this); }           // Prefix
      iterator  operator -- (int) { --_node; return iterator(_node+1); } // Postfix

      iterator  operator + (int i) const { return iterator( _node + i ); }
      iterator& operator += (int i) { _node += i; return (*this); }

      iterator& operator = (const iterator& i) { this->_node = i._node; return (*this); }

      bool operator != (const iterator& i) const { return (this->_node != i._node); }
      bool operator == (const iterator& i) const { return (this->_node == i._node); }

   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator( _data ); }
   iterator end()   const { return iterator( _data + _size ); }
   bool empty()     const { return ( _size == 0 ); }
   size_t size()    const { return _size; }

   T& operator [] (size_t i) { return _data[i]; }
   const T& operator [] (size_t i) const { return _data[i]; }

   void push_back(const T& x) {
      if(_size >= _capacity){
         if(_capacity == 0) _capacity = 1;
         else _capacity <<= 1;
         T* temp = new T[_capacity];
         for(unsigned int i = 0 ; i < _size ; ++i )
            temp[i] = _data[i];
         delete []_data;
         _data = temp;
      }
      _data[_size++] = x;
      _isSorted = false;
      return;
   }
   void pop_front() { if(!empty()){ _data[0] = _data[(--_size)]; _isSorted = false; } return; }
   void pop_back()  { if(!empty()) --_size; return; }

   bool erase(iterator pos) {
      if( empty() || &(*pos) < _data || &(*pos) >= _data+_size ){
         return false;
      }
      (*pos) = _data[(--_size)];
      _isSorted = false;
      return true;
   }
   bool erase(const T& x) {
      for(unsigned int i = 0 ; i < _size ; ++i ){
         if( _data[i] == x ){
            _data[i] = _data[(--_size)];
            _isSorted = false;
            return true;
         }
      }
      return false;
   }

   void clear() { _size = 0; _isSorted = true; }

   // This is done. DO NOT change this one.
   void sort() const{
      if( !empty() && !_isSorted ){
         ::sort(_data, _data+_size);
         _isSorted = true;
      }
      return;
   }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
};

#endif // ARRAY_H

