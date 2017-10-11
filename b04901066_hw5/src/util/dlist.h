/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() : _isSorted(true) {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T&  operator *  ()const{ return *(this); }
      T&        operator *  ()     { return _node->_data; }
      iterator& operator ++ ()     { _node = _node->_next; return *(this); }
      iterator  operator ++ (int)  { _node = _node->_next; return iterator(_node->_prev); }
      iterator& operator -- ()     { _node = _node->_prev; return *(this); }
      iterator  operator -- (int)  { _node = _node->_prev; return iterator(_node->_next); }

      iterator& operator = (const iterator& i) { this->_node = i._node; return (*this); }

      bool operator != (const iterator& i) const { return (this->_node != i._node); }
      bool operator == (const iterator& i) const { return (this->_node == i._node); }

   private:
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const { return iterator( _head ); }        //_head_node
   iterator end()   const { return iterator( _head->_prev ); } //dummy_node
   bool     empty() const { return (_head == _head->_next); }
   size_t   size()  const {
      size_t i = 0;
      DListNode<T>* c = _head;
      while( c->_next != _head ){
         c = c->_next;
         ++i;
      }
      return i;
   }

   void push_back(const T& x){
       (_head->_prev)->_prev = \
      ((_head->_prev->_prev)->_next = \
       new DListNode<T>( x, _head->_prev->_prev, _head->_prev));
      _isSorted = false;
      return;
   }
   void pop_front(){
      if(_head->_next != _head){
         DListNode<T>* pop_temp = _head ;
         _head = _head->_next;           //new_head_node
         Extract_Node(pop_temp);
         delete pop_temp;
      }
      return;
   }
   void pop_back(){
      if(_head->_next != _head){
         DListNode<T>* pop_temp = _head->_prev->_prev ;
         Extract_Node(pop_temp);
         _head = pop_temp->_next->_next;
         delete pop_temp;
      }
      return;
   }

   // return false if nothing to erase
   bool erase(iterator pos){
      DListNode<T>* temp = _head;
      while( temp->_next != _head ){
         if( temp == pos._node ){
            if( temp == _head ) _head = _head->_next;
            Extract_Node(temp);
            delete temp;
            return true;
         }
         temp = temp->_next;
      }
      return false;
   }
   bool erase(const T& x){
      DListNode<T>* temp = _head;
      while( temp->_next != _head ){
         if( temp->_data == x ){
            if( temp == _head ) _head = _head->_next;
            Extract_Node(temp);
            delete temp;
            return true;
         }
         temp = temp->_next;
      }
      return false;
   }

   void clear(){
      DListNode<T>* origin_head = _head;
      while( _head->_next != origin_head ){
         _head = _head->_next;
         delete _head->_prev;
      }
      _head->_prev = _head->_next = _head;
      _isSorted = true;
      return;
   }  // delete all nodes except for the dummy node

   void sort(){
      if(!_isSorted){
         DListNode<T>* sort_i = _head;
         while( (sort_i->_next)!=_head ){    // i from _head to dummy
            DListNode<T>* sort_j = sort_i;   // j = i

                                             // j from i to _head||<=i
            while(sort_j!=_head && sort_i->_data < (sort_j->_prev)->_data)
               sort_j = sort_j->_prev;       // --j

            DListNode<T>* insert_i = sort_i;
            sort_i = sort_i->_next;          // ++i

            if(insert_i != sort_j){          // need to swap
               Extract_Node(insert_i);
               Insert_Node(insert_i, sort_j);
               if(sort_j == _head)_head = insert_i; // insert at head
            }

         }
         _isSorted = true;
      }
   }

private:
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] helper functions; called by public member functions
   //_head***e***dummy
   void Extract_Node(DListNode<T>* e){
      (e->_next)->_prev = e->_prev;
      (e->_prev)->_next = e->_next;
      return;
   }
   //_head***ij***dummy
   void Insert_Node(DListNode<T>* i_, DListNode<T>* _j){
      i_->_next = _j;
      i_->_prev = _j->_prev;
     (_j->_prev)->_next = i_;
      _j->_prev = i_;
      return;
   }
};

#endif // DLIST_H

