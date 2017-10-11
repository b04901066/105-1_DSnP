#include "p2Table.h"
#include <iostream>
#define nullnum 128
using namespace std;

// Implement member functions of class Row and Table here

bool Table::read(const string& csvFile){
  fstream file;
  int rows = 0 ;
  col_num = 1;
  string str;

  file.open( csvFile.c_str() , ios::in);
  if(!file)return false;
  getline( file , str , '\r' );
  for( int i=0 ; str[i] != '\0' ; ++i ){
    if( str[i] == ',' ){
      ++col_num;
    }
  }
  str.clear();
  file.seekg( 0 , ios::beg );

  getline( file , str );
  int endcheck = 0;
  for(int i=0 ; str[i] != '\0' ; ++i ){
    if( str[i] == '\r' ){
      ++rows;
      endcheck = i;
    }
  }
  if(endcheck>0)if(str[endcheck-1] == '\r')--rows;

  str.clear();
  file.seekg( 0 , ios::beg );
  
  for(int r=0;r<rows;++r){
    int* coldata = new (nothrow) int[col_num];
    Row row(coldata);
    for(int c=0 ; c<col_num ; ++c ){
      string str_temp;
      if(c<col_num-1)getline( file , str_temp , ',' );
      else getline( file , str_temp , '\r' );
      if( str_temp.length() == 0 )coldata[c] = nullnum;
      else {
        stringstream ss(str_temp);
        ss >> coldata[c];
      }
    }
    _rows.push_back(row);
  }

  file.close();
  return true; // TODO
}

void Table::print(){
  for(unsigned int r = 0 ; r < (_rows.size()) ; ++r){
    for(int c = 0 ; c < col_num ; ++c ){
      if(_rows[r][c]==nullnum)cout<<setw(4)<<right<<' ';
      else cout<<setw(4)<<right<<_rows[r][c];
    }
    cout<<endl;
  }
  return;
}
void Table::sum(){
  int choose_col = 0;
  cin >> choose_col;
  if(cin.fail()){
    cout<<"Input error!"<<endl;
    return;
  }
  if(choose_col<0 || choose_col >= col_num){
    cout<<"Column #"<< choose_col <<" does not exist."<< endl;
    return;
  }
  int sum = 0;
  for(unsigned int r = 0 ; r < (_rows.size()) ; ++r){
    if(_rows[r][choose_col] <= 100 && _rows[r][choose_col] >= -99)sum += _rows[r][choose_col];
  }
  cout<<"The summation of data in column #"<< choose_col <<" is "<< sum <<'.'<< endl;
  return;
}
void Table::ave(){
  int choose_col = 0;
  cin >> choose_col;
  if(cin.fail()){
    cout<<"Input error!"<<endl;
    return;
  }
  if(choose_col<0 || choose_col >= col_num){
    cout<<"Column #"<< choose_col <<" does not exist."<< endl;
    return;
  }
  double sum = 0.0;
  int mun = 0;
  for(unsigned int r = 0 ; r < (_rows.size()) ; ++r){
    if(_rows[r][choose_col] <= 100 && _rows[r][choose_col] >= -99){
      ++mun;
      sum += _rows[r][choose_col];
    }
  }
  cout<<"The average of data in column #"<< choose_col <<" is "<< fixed << setprecision(1)<< (sum/mun) <<'.'<< endl;
  return;
}
void Table::max(){
  int choose_col = 0;
  cin >> choose_col;
  if(cin.fail()){
    cout<<"Input error!"<<endl;
    return;
  }
  if(choose_col<0 || choose_col >= col_num){
    cout<<"Column #"<< choose_col <<" does not exist."<< endl;
    return;
  }
  int max = -99;
  for(unsigned int r = 0 ; r < (_rows.size()) ; ++r){
    if(_rows[r][choose_col] <= 100 && _rows[r][choose_col] >= -99){
      if((_rows[r][choose_col])>max)max = _rows[r][choose_col];
    }
  }
  cout<<"The maximum of data in column #"<< choose_col <<" is "<< max <<'.'<< endl;
  return;
}
void Table::min(){
  int choose_col = 0;
  cin >> choose_col;
  if(cin.fail()){
    cout<<"Input error!"<<endl;
    return;
  }
  if(choose_col<0 || choose_col >= col_num){
    cout<<"Column #"<< choose_col <<" does not exist."<< endl;
    return;
  }
  int min = 100;
  for(unsigned int r = 0 ; r < (_rows.size()) ; ++r){
    if(_rows[r][choose_col] <= 100 && _rows[r][choose_col] >= -99){
      if((_rows[r][choose_col])<min)min = _rows[r][choose_col];
    }
  }
  cout<<"The minimum of data in column #"<< choose_col <<" is "<< min <<'.'<< endl;
  return;
}
void Table::count(){
  int choose_col = 0;
  cin >> choose_col;
  if(cin.fail()){
    cout<<"Input error!"<<endl;
    return;
  }
  if(choose_col<0 || choose_col >= col_num){
    cout<<"Column #"<< choose_col <<" does not exist."<< endl;
    return;
  }
  int count = 0;
  for(unsigned int r = 0 ; r < (_rows.size()) ; ++r){
    if(_rows[r][choose_col] <= 100 && _rows[r][choose_col] >= -99){
      unsigned int i = 0;
      for( ; i < r ; ++i){
        if(_rows[i][choose_col] <= 100 && _rows[i][choose_col] >= -99){
          if(_rows[i][choose_col] == _rows[r][choose_col])break;
        }
      }
      if(i == r)++count;
    }
  }
  cout<<"The distinct count of data in column #"<< choose_col <<" is "<< count <<'.'<<endl;
  return;
}
void Table::add(){
  int* coldata = new (nothrow) int[col_num];
  Row new_row(coldata);
  for(int c=0 ; c<col_num ; ++c ){
    string s;
    stringstream ss;
    cin >> s;
    if( s == "-")coldata[c] = nullnum;
    else{
      ss << s;
      ss >> coldata[c];
      if(ss.fail() || coldata[c]>100 || coldata[c]<-99)coldata[c] = nullnum;
    }
  }
  _rows.push_back(new_row);
  return;
}
