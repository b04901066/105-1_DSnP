/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef BST_H
#define BST_H

using namespace std;
template <class T> class BSTree;
template <class T> class BSTreeNode
{
   // TODO: design your own class!!
   friend class BSTree<T>;
   friend class BSTree<T>::iterator;
   BSTreeNode(const T& d): _data(d), _left(NULL), _right(NULL) {}

   T               _data;
   BSTreeNode<T>*  _left;
   BSTreeNode<T>*  _right;
};

template <class T> class BSTree
{
public:
    BSTree(){ _root = NULL; }
   ~BSTree(){ clear(); }
   // TODO: design your own class!!
   class iterator
   {
      friend class BSTree;
   public:
       iterator(){}
       iterator(const iterator& i) : _trace(i._trace) {}
      ~iterator(){} // Should NOT delete _trace.top()

      // TODO: implement these overloaded operators
      T& operator * () { return _trace.top()->_data; }
      iterator& operator ++ (){
         if( _trace.top() == NULL );
         else if( _trace.top()->_right != NULL ){
            _trace.push(_trace.top()->_right);
            while( _trace.top()->_left != NULL )_trace.push(_trace.top()->_left);
         }
         else{
            while(true){
               BSTreeNode<T>* cur = _trace.top(); _trace.pop();
               if(_trace.empty()){//_trace = end()._trace
                  BSTreeNode<T>* run_r = cur;
                  while(run_r != NULL){
                     _trace.push(run_r);
                     run_r = run_r->_right;
                  }
                  _trace.push(NULL);//end()._trace.top() is always NULL
                  break;
               }
               else if( _trace.top()->_left == cur )break;
            }
         }
         return *(this);
      }
      iterator  operator ++ (int){ iterator temp(*(this)); ++(*this); return temp; }
      iterator& operator -- (){
         if( _trace.empty() );
         else if( _trace.top() == NULL ){
            _trace.pop();//for --end()
            if( _trace.empty() )_trace.push(NULL);
         }
         else if( _trace.top()->_left != NULL ){
            _trace.push(_trace.top()->_left);
            while( _trace.top()->_right != NULL )_trace.push(_trace.top()->_right);
         }
         else{
            while(true){
               BSTreeNode<T>* cur = _trace.top(); _trace.pop();
               if(_trace.empty()){//this->_trace = --begin()._trace
                  BSTreeNode<T>* run_l = cur;
                  while(run_l != NULL){
                     _trace.push(run_l);
                     run_l = run_l->_left;
                  }
                  _trace.push(NULL);
                  break;
               }
               else if( _trace.top()->_right == cur )break;
            }
         }
         return *(this);
      }
      iterator  operator -- (int){ iterator temp(*(this)); --(*this); return temp; }

      iterator& operator = (const iterator& i) { this->_trace = i._trace; return *(this); }
      bool operator != (const iterator& i) const { return (this->_trace.top() != i._trace.top()); }
      bool operator == (const iterator& i) const { return (this->_trace.top() == i._trace.top()); }
   private:
      stack< BSTreeNode<T>* >  _trace;
   };
   iterator begin() const {
      iterator temp;
      BSTreeNode<T>* run_l = _root;
      if(run_l == NULL) temp._trace.push(NULL);
      while(run_l != NULL){
         temp._trace.push(run_l);
         run_l = run_l->_left;
      }
      return temp;
   }
   iterator end() const {
      iterator temp;
      BSTreeNode<T>* run_r = _root;
      while(run_r != NULL){
         temp._trace.push(run_r);
         run_r = run_r->_right;
      }
      temp._trace.push(NULL);//end()._trace.top() is always NULL
      return temp;
   }
   bool empty() const { return ( _root == NULL ); }
   size_t size() const {return size(_root);}
   void insert(const T& x){
      if(_root == NULL){ _root = new BSTreeNode<T>(x); return; }
      BSTreeNode<T>* _in = _root;//_root != NULL
      while(true){
         if(x < _in->_data){
            if( _in->_left  == NULL ){ _in->_left  = new BSTreeNode<T>(x); return; }
            else _in = _in->_left;
         }
         else {
            if( _in->_right == NULL ){ _in->_right = new BSTreeNode<T>(x); return; }
            else _in = _in->_right;
         }
      }
   }
   void print(){ if(_root != NULL)print(_root, 0); return; }
   // return false if nothing to erase
   bool erase(iterator pos){
      if( pos._trace.empty() || pos._trace.top() == NULL )return false;

      BSTreeNode<T>* _del = pos._trace.top();
      if( _del->_left == NULL && _del->_right == NULL ){
         pos._trace.pop();
         if(pos._trace.empty())_root = NULL;
         else if(pos._trace.top()->_left == _del) pos._trace.top()->_left = NULL;
         else pos._trace.top()->_right = NULL;
      }
      else if( _del->_left == NULL || _del->_right == NULL){
         pos._trace.pop();
         if(_del->_left != NULL){
            if(pos._trace.empty())_root = _del->_left;
            else if(pos._trace.top()->_left == _del)pos._trace.top()->_left = _del->_left;
            else pos._trace.top()->_right = _del->_left;
         }
         else{
            if(pos._trace.empty())_root = _del->_right;
            else if(pos._trace.top()->_left == _del)pos._trace.top()->_left = _del->_right;
            else pos._trace.top()->_right = _del->_right;
         }
      }
      else{
         iterator rpl(pos); ++rpl;
         BSTreeNode<T>* rmv = rpl._trace.top(); rpl._trace.pop();
         if(rmv->_right != NULL){
            if(rpl._trace.top()->_left == rmv)rpl._trace.top()->_left = rmv->_right;
            else rpl._trace.top()->_right = rmv->_right;
         }
         else{
            if(rpl._trace.top()->_left == rmv)rpl._trace.top()->_left = NULL;
            else rpl._trace.top()->_right = NULL;
         }
         pos._trace.pop();
         if(pos._trace.empty())_root = rmv;
         else if(pos._trace.top()->_left == _del)pos._trace.top()->_left  = rmv;
         else                                    pos._trace.top()->_right = rmv;
         rmv->_left  = _del->_left;
         rmv->_right = _del->_right;
      }
      delete _del;
      return true;
   }
   bool erase(const T& x){
      if(empty())return false;
      iterator pos;
      pos._trace.push(_root);
      while(pos._trace.top() != NULL){
         if(x == pos._trace.top()->_data) return erase(pos);
         else if(x < pos._trace.top()->_data) pos._trace.push(pos._trace.top()->_left);
         else pos._trace.push(pos._trace.top()->_right);
      }
      return false;
   }
   void pop_front(){ erase(begin()); return; }
   void pop_back() { erase(--end()); return; }
   void clear(){ clear(_root); _root = NULL; }
   void sort(){}    //dummy function

private:
   BSTreeNode<T>*  _root;

   //recursive functions
   size_t size(BSTreeNode<T>* _p) const {
      if( _p == NULL ) return 0;
      else return (size(_p->_left)+size(_p->_right)+1);
   }
   void print(BSTreeNode<T>* _p,unsigned int deep){
      for(unsigned int i = 0 ; i < deep ; ++i) cout << ' ';
      if( _p == NULL ){ cout << "[0]" << endl; return; }
      cout << _p->_data << endl;
      print(_p->_left , deep+2);
      print(_p->_right, deep+2);
      return;
   }
   void clear(BSTreeNode<T>* _p){ // delete all nodes
      if( _p == NULL ) return;
      clear(_p->_left); clear(_p->_right);
      delete _p; return;
   }
};

#endif // BST_H

