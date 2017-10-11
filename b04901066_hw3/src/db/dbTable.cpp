/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include "dbTable.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   size_t nr = r.size();
   for (size_t i = 0; i < nr ; ++i){
      if(r[i] == INT_MAX)os << '.';
      else os << r[i];
      if(i<(nr-1))os << ' ';
   }
   return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells are printed as '.'
   size_t nr = t.nRows(), nc = t.nCols();
   for (size_t i = 0; i < nr ; ++i){
      for (size_t j = 0; j < nc ; ++j){
         if(t[i][j] != INT_MAX)os << setw(6) << right << t[i][j];
         else os << setw(6) << right << '.';
      }
      os << endl;
   }
   return os;
}

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume all the data of the table are in a single line.
   size_t row_num = 0 , col_num = 1 , endcheck = 0 ;
   string Input_Table;
   getline( ifs , Input_Table );
   for (size_t i = 0; Input_Table[i] != '\r'; ++i)
      if( Input_Table[i] == ',' ) ++col_num;

   for (size_t i = 0; i < Input_Table.length() ; ++i)
      if( Input_Table[i] == '\r' ){
         ++row_num;
         endcheck = i;
      }
   if(endcheck>0)if(Input_Table[endcheck-1] == '\r')--row_num;

   size_t cursor_pos1 = 0 , cursor_pos2 = 0;
   for(size_t r = 0 ; r < row_num ; ++r){
      vector<int> add_row;
      for(size_t c = 0 ; c < col_num ; ++c){
         string temp_str;
         cursor_pos2 = Input_Table.find_first_of(",\r", cursor_pos1);
         temp_str = Input_Table.substr(cursor_pos1, (cursor_pos2 - cursor_pos1));
         cursor_pos1 = (cursor_pos2+1);
         if( temp_str.length() == 0 )add_row.push_back(INT_MAX);
         else {
            size_t add_num;
            stringstream ss;
            ss << temp_str;
            ss >> add_num;
            add_row.push_back(add_num);
         }
      }
      t._table.push_back(DBRow(add_row));
   }
   return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void
DBRow::removeCell(size_t c)
{
   // TODO
   _data.erase(_data.begin()+c);
   return;
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder
   size_t idx = _sortOrder.size();
   for(size_t i = 0 ; i < idx ; ++i){
      if( r1[_sortOrder[i]] < r2[_sortOrder[i]] )return true;
      if( r1[_sortOrder[i]] > r2[_sortOrder[i]] )return false;
   }
   return false;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void
DBTable::reset()
{
   // TODO
   vector<DBRow>tmp; tmp.swap(_table);
}

void
DBTable::addCol(const vector<int>& d)
{
   // TODO: add a column to the right of the table. Data are in 'd'.
   size_t dn = d.size() , nr = this->nRows();
   for(size_t r = 0 ; r < nr ; ++r ){
      _table[r].addData(( r<dn ? d[r] : INT_MAX ));
   }
   return;
}

void
DBTable::delRow(int c)
{
   // TODO: delete row #c. Note #0 is the first row.
   _table.erase(_table.begin()+c);
   return;
}

void
DBTable::delCol(int c)
{
   // delete col #c. Note #0 is the first row.
   for (size_t i = 0, n = _table.size(); i < n; ++i)
      _table[i].removeCell(c);
   return;
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in column #c are null, return NAN
// - Return "float" because NAN is a float.
float
DBTable::getMax(size_t c) const
{
   // TODO: get the max data in column #c
   float maxnum = NAN;
   size_t nr = this->nRows();
   for(size_t r = 0 ; r < nr ; ++r)
      if( _table[r][c] != INT_MAX ){
         if( (maxnum == maxnum) == 0 ) maxnum = _table[r][c];
         if( maxnum < _table[r][c] ) maxnum = _table[r][c];
      }
   return maxnum;
}

float
DBTable::getMin(size_t c) const
{
   // TODO: get the min data in column #c
   float minnum = NAN;
   size_t nr = this->nRows();
   for(size_t r = 0 ; r < nr ; ++r)
      if( _table[r][c] != INT_MAX ){
         if((minnum == minnum) == 0 ) minnum = _table[r][c];
         if( minnum > _table[r][c] )minnum = _table[r][c];
      }
   return minnum;
}

float
DBTable::getSum(size_t c) const
{
   // TODO: compute the sum of data in column #c
   float summun = NAN;
   size_t nr = this->nRows();
   for(size_t r = 0 ; r < nr ; ++r)
      if( _table[r][c] != INT_MAX ){
         if((summun == summun) == 0 )summun = _table[r][c];
         else summun += _table[r][c];
      }
   return summun;
}

int
DBTable::getCount(size_t c) const
{
   // TODO: compute the number of distinct data in column #c
   // - Ignore null cells
   int distinctmun = 0;
   size_t nr = this->nRows();
   for(size_t r = 0 ; r < nr ; ++r){
      if( _table[r][c] == INT_MAX )continue;
      size_t i = 0;
      for( ; i < r ; ++i)if( _table[i][c] == _table[r][c] )break;
      if(i == r)++distinctmun;
   }
   return distinctmun;
}

float
DBTable::getAve(size_t c) const
{
   // TODO: compute the average of data in column #c
   float summun = NAN;
   size_t nr = this->nRows(), validmun = 0;
   for(size_t r = 0 ; r < nr ; ++r)
      if( _table[r][c] != INT_MAX ){
         ++validmun;
         if((summun == summun) == 0 )summun = _table[r][c];
         else summun += _table[r][c];
      }
   if(((summun == summun) == 0 ) || validmun == 0 )return NAN;
   else return (summun/validmun);
}

void
DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
   std::sort(_table.begin(), _table.end(), s);
}

void
DBTable::printCol(size_t c) const
{
   // TODO: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   size_t nr = this->nRows();
   for(size_t r = 0 ; r < nr ; ++r){
      printData(cout, _table[r][c]);
      if(r == (nr-1))cout << endl;
      else cout << ' ';
   }
}

void
DBTable::printSummary() const
{
   size_t nr = nRows(), nc = nCols(), nv = 0;
   for (size_t i = 0; i < nr; ++i)
      for (size_t j = 0; j < nc; ++j)
         if (_table[i][j] != INT_MAX) ++nv;
   cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
        << nv << ")" << endl;
}
