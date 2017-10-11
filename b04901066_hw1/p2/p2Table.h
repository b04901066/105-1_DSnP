#ifndef P2_TABLE_H
#define P2_TABLE_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

using namespace std;

class Row
{
public:
   Row(int* col_data){ _data = col_data;}
   const int& operator[] (size_t i) const {return _data[i];} // TODO
   int& operator[] (size_t i) {return _data[i];} // TODO

private:
   int* _data;
};

class Table
{
public:
   ~Table(){for(int i = 0 ; i < col_num ; ++i) delete [] &(_rows[i][0]); }
   const Row& operator[] (size_t i) const{return _rows[i];}
   Row& operator[] (size_t i){return _rows[i];}

   bool read(const string&);
   void print();
   void sum();
   void ave();
   void max();
   void min();
   void count();
   void add();

private:
   vector<Row>  _rows;
   int col_num;
};

#endif // P2_TABLE_H
