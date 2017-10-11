/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <ctime>
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
//static char buf[1024];
static string buf;
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   ifstream file;
   string   token;
   size_t   cut = 0;
   lineNo = colNo = 0;

   file.open( fileName.c_str() , ios::in);
   if(!file){
      cerr << "Cannot open design \"" << fileName << "\"!!" << endl;
      return false;
   }

   //header
   if(!getline( file , buf )){
      errMsg = "aag";
      return parseError(MISSING_IDENTIFIER);
   }
   for(int ar = 0 ; ar < 6 ; ++ar){
      cut = buf.find(' ');
      if(buf.find('\t') < cut){
         colNo += buf.find('\t');
         errInt='\t';
         return parseError(ILLEGAL_WSPACE);
      }
      if(cut == string::npos){//last token
         if(ar<5){
            colNo += buf.length();
            if(buf.empty()){
               errMsg = "number of variables";
               return parseError(MISSING_NUM);
            }
            else return parseError(MISSING_SPACE);
         }
         token = buf;
         buf.clear();
      }
      else{
         if(ar==5){
            colNo += cut;
            if(cut==0) return parseError(EXTRA_SPACE);
            else return parseError(MISSING_NEWLINE);
         }
         token = buf.substr(0, cut);
         buf   = buf.substr(cut+1);
      }
      if(token.empty()) return parseError(EXTRA_SPACE);
      if(ar==0){
         if( token.compare("aag") != 0 ){
            errMsg = token;
            return parseError(ILLEGAL_IDENTIFIER);
         }
      }
      else if(ar==1){
         if(!myStr2Unsigned(token, M)){
            errMsg = "number of variables(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==2){
         if(!myStr2Unsigned(token, I)){
            errMsg = "number of PIs(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==3){
         if(!myStr2Unsigned(token, L)){
            errMsg = "number of latches(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==4){
         if(!myStr2Unsigned(token, O)){
            errMsg = "number of POs(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==5){
         if(!myStr2Unsigned(token, A)){
            errMsg = "number of AIGs(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      colNo += (token.length()+1);
   }
   ++lineNo;

   //check
   if( M < I + L + A ){
      errMsg = "Number of variables";
      errInt = M;
      return parseError(NUM_TOO_SMALL);
   }
   if( L > 0 ){
      errMsg = "latches";
      return parseError(ILLEGAL_NUM);
   }

   //M
   Gate_List = new CirGate[M+O+1];  //M(PI+AIG) PO const
   Gate_List[0].type = CONST_GATE;
   Gate_List[0].def_line = 0;

   //I <LID>
   for(unsigned i = 0 ; i < I; ++i){
      unsigned literal_ID;
      colNo = 0;
      //EOF
      if(!getline( file , buf )){
         errMsg = "PI";
         return parseError(MISSING_DEF);
      }
      //empty string
      if(buf.empty()){
         errMsg = "PI literal ID";
         return parseError(MISSING_NUM);
      }
      //EXTRA_SPACE
      if(buf.find(' ') != string::npos){
         colNo += buf.find(' ');
         if(buf.find(' ') == 0)return parseError(EXTRA_SPACE);
         else return parseError(MISSING_NEWLINE);
      }
      //ILLEGAL PI
      if(!myStr2Unsigned(buf, literal_ID)){
         errMsg = "PI literal ID(" + buf + ")";
         return parseError(ILLEGAL_NUM);
      }
      errInt = literal_ID;
      //const
      if(literal_ID/2 == 0) return parseError(REDEF_CONST);
      //exceed MAX
      if(literal_ID/2 > M) return parseError(MAX_LIT_ID);
      //invert
      if(literal_ID%2){
         errMsg = "PI";
         return parseError(CANNOT_INVERTED);
      }
      //redefine
      if(Gate_List[literal_ID/2].type != UNDEF_GATE){
         errGate = &(Gate_List[literal_ID/2]);
         return parseError(REDEF_GATE);
      }

      //write Gate
      Gate_List[literal_ID/2].type = PI_GATE;
      Gate_List[literal_ID/2].def_line = ++lineNo;
      Input_VIdList.push_back(literal_ID/2);
   }

   //L <LID> <LID>
   for(unsigned i = 0 ; i < L; ++i){
      colNo = 0;
      if(!getline( file , buf )){
         errMsg = "Latche literal ID";
         return parseError(MISSING_NUM);
      }
      ++lineNo;
   }

   //O <LID>
   for(unsigned i = 0 ; i < O; ++i){
      unsigned literal_ID;
      colNo = 0;
      //EOF
      if(!getline( file , buf )){
         errMsg = "PO";
         return parseError(MISSING_DEF);
      }
      //empty string
      if(buf.empty()){
         errMsg = "PO literal ID";
         return parseError(MISSING_NUM);
      }
      //EXTRA_SPACE
      if(buf.find(' ') != string::npos){
         colNo += buf.find(' ');
         if(buf.find(' ') == 0)return parseError(EXTRA_SPACE);
         else return parseError(MISSING_NEWLINE);
      }
      //ILLEGAL PO
      if(!myStr2Unsigned(buf, literal_ID)){
         errMsg = "PO literal ID(" + buf + ")";
         return parseError(ILLEGAL_NUM);
      }
      errInt = literal_ID;
      //exceed MAX
      if(literal_ID/2 > M)return parseError(MAX_LIT_ID);

      //write Gate
      Gate_List[M+1+i].type = PO_GATE;
      Gate_List[M+1+i].def_line = ++lineNo;
      Gate_List[M+1+i].fanin__LIDList.push_back(literal_ID);
      Gate_List[literal_ID/2].fanout_LIDList.push_back(2*(M+1+i)+(literal_ID%2));
   }

   //A <LHS> <RHS1> <RHS2>
   for(unsigned i = 0 ; i < A; ++i){
      unsigned literal_ID, LHS, RHS1, RHS2;
      colNo = 0;
      //EOF
      if(!getline( file , buf )){
         errMsg = "AIG";
         return parseError(MISSING_DEF);
      }
      //empty string
      if(buf.empty()){
         errMsg = "AIG literal ID";
         return parseError(MISSING_NUM);
      }
      for(int ar = 0 ; ar < 3 ; ++ar){
         cut = buf.find(' ');
         if(buf.find('\t') < cut){
            colNo += buf.find('\t');
            errInt='\t';
            return parseError(ILLEGAL_WSPACE);
         }
         if(cut == string::npos){//last token
            if(ar<2){
               colNo += buf.length();
               if(buf.empty()){
                  errMsg = "number of variables";
                  return parseError(MISSING_NUM);
               }
               else return parseError(MISSING_SPACE);
            }
            token = buf;
            buf.clear();
         }
         else{
            if(ar==2){
               colNo += cut;
               if(cut==0) return parseError(EXTRA_SPACE);
               else return parseError(MISSING_NEWLINE);
            }
            token = buf.substr(0, cut);
            buf   = buf.substr(cut+1);
         }
         if(token.empty())return parseError(EXTRA_SPACE);
         //ILLEGAL AIG
         if(!myStr2Unsigned(token, literal_ID)){
            errMsg = "AIG literal ID(" + buf + ")";
            return parseError(ILLEGAL_NUM);
         }
         errInt = literal_ID;
         //const
         if(ar == 0 && literal_ID/2 == 0) return parseError(REDEF_CONST);
         //exceed MAX
         if(literal_ID/2 > M) return parseError(MAX_LIT_ID);
         //invert
         if(ar == 0 && literal_ID%2){
            errMsg = "AIG";
            return parseError(CANNOT_INVERTED);
         }
         //redefine
         if(ar == 0 && Gate_List[literal_ID/2].type != UNDEF_GATE){
            errGate = &(Gate_List[literal_ID/2]);
            return parseError(REDEF_GATE);
         }
              if(ar==0) LHS  = literal_ID;
         else if(ar==1) RHS1 = literal_ID;
         else if(ar==2) RHS2 = literal_ID;
         colNo += (token.length()+1);
      }
      //write Gate
      Gate_List[LHS/2].type = AIG_GATE;
      Gate_List[LHS/2].def_line = ++lineNo;
      Gate_List[LHS/2].fanin__LIDList.push_back(RHS1);
      Gate_List[RHS1/2].fanout_LIDList.push_back(LHS+(RHS1%2));
      Gate_List[LHS/2].fanin__LIDList.push_back(RHS2);
      Gate_List[RHS2/2].fanout_LIDList.push_back(LHS+(RHS2%2));
   }

   //Symbol [ilo]<pos> <string>
   while(getline( file , buf )){
      colNo = 0;
      char ilo;
      unsigned pos;
      string name;

      if(buf.empty()){
         errMsg = "symbolic name";
         return parseError(MISSING_IDENTIFIER);
      }
      if(buf[0]=='c'){//comment
         if(buf.size() > 1){
            colNo = 1;
            return parseError(MISSING_NEWLINE);
         }
         break;
      }
      cut = buf.find(' ');
      if(buf.find('\t') != string::npos){
         colNo += buf.find('\t');
         errInt='\t';
         return parseError(ILLEGAL_WSPACE);
      }
      if(cut != string::npos){
         name = buf.substr(cut+1);
         buf  = buf.substr(0,cut);
      }
      if(buf.empty())return parseError(EXTRA_SPACE);
      if( buf[0]=='i' || buf[0]=='o' )ilo = buf[0];
      else {
         errMsg = buf[0];
         return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      buf = buf.substr(1);
      ++colNo;
      if(buf.empty())return parseError(EXTRA_SPACE);
      //ILLEGAL
      if(!myStr2Unsigned(buf, pos)){
         errMsg = "symbol index(" + buf + ")";
         return parseError(ILLEGAL_NUM);
      }
      //exceed MAX & redefine
      if(ilo == 'i'){
         if(pos >= I){
            errMsg = "PI index";
            errInt = pos;
            return parseError(NUM_TOO_BIG);
         }
         if(Gate_List[Input_VIdList[pos]].symbol != NULL){
            errMsg = "i";
            errInt = pos;
            return parseError(REDEF_SYMBOLIC_NAME);
         }
      }
      if(ilo == 'o'){
         if(pos >= O){
            errMsg = "PO index";
            errInt = pos;
            return parseError(NUM_TOO_BIG);
         }
         if(Gate_List[M+1+pos].symbol != NULL){
            errMsg = "o";
            errInt = pos;
            return parseError(REDEF_SYMBOLIC_NAME);
         }
      }
      colNo += buf.length()+1;
      //ILLEGAL
      if(name.empty()){
         errMsg = "symbolic name";
         return parseError(MISSING_IDENTIFIER);
      }
      for(unsigned re = 0; re < name.length() ; ++colNo, ++re){
         if(name[re] < 0x20){
            errInt = name[re];
            return parseError(ILLEGAL_SYMBOL_NAME);
         }
      }
      //write
      if(ilo == 'i') Gate_List[Input_VIdList[pos]].symbol = new string(name);
      if(ilo == 'o') Gate_List[M+1+pos].symbol = new string(name);
      ++lineNo;
   }
   file.close();
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics" << endl
        << "==================" << endl
        << "  PI   "  <<  right << setw(11) << I << endl
        << "  PO   "  <<  right << setw(11) << O << endl
        << "  AIG  "  <<  right << setw(11) << A << endl
        << "------------------" << endl
        << "  Total"  <<  right << setw(11) << I+O+A << endl;
   return;
}

void
CirMgr::printNetlist() const
{
   print_idx(true);
   ++DFS_refe;
   for(unsigned i = 0 ; i < O; ++i)
      print_DFS(Gate_List[M+1+i], DFS_refe);
   cout << endl;
   return;
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   unsigned s = Input_VIdList.size();
   for(unsigned i = 0 ; i < s; ++i){
      cout << ' ' << Input_VIdList[i];
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(unsigned i = M+1 ; i < (M+O+1); ++i)cout << ' ' << i;
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   bool f_found = false, nu_found = false;
   for(unsigned i = 0 ; i < (M+O+1); ++i){
      for(unsigned j = 0 ; j < Gate_List[i].fanin__LIDList.size(); ++j){
         if(Gate_List[Gate_List[i].fanin__LIDList[j]/2].type == UNDEF_GATE){
            if(!f_found){
               cout << "Gates with floating fanin(s):";
               f_found = true;
            }
            cout << ' ' << i;//UNDEF
            break;
         }
      }
   }
   if(f_found)cout << endl;

   for(unsigned i = 0 ; i < (M+1); ++i){
      if(Gate_List[i].type == PI_GATE || Gate_List[i].type == AIG_GATE){
         if(Gate_List[i].fanout_LIDList.empty()){
            if(!nu_found){
               cout << "Gates defined but not used  :";
               nu_found = true;
            }
            cout << ' ' << i;//no fanout
         }
      }
   }
   if(nu_found)cout << endl;
   return;
}

void
CirMgr::writeAag(ostream& outfile) const
{
   unsigned new_A = 0;
   ++DFS_refe;
   for(unsigned i = 0 ; i < O; ++i)
      write_DFS_A( outfile, Gate_List[M+1+i], new_A, DFS_refe, true);

   //header
   outfile <<  "aag "  << M << ' ' << I << ' '\
           << L << ' ' << O << ' ' << new_A << endl;
   //PI
   for(unsigned i = 0 ; i < I; ++i)
      outfile << (2*Input_VIdList[i]) << endl;
   //Latch
   for(unsigned i = 0 ; i < L; ++i)
      outfile << endl;
   //PO
   for(unsigned i = 0 ; i < O; ++i)
      outfile << (Gate_List[M+1+i].fanin__LIDList[0]) << endl;
   //A
   ++DFS_refe;
   for(unsigned i = 0 ; i < O; ++i)
      write_DFS_A( outfile, Gate_List[M+1+i], new_A, DFS_refe, false);

   //PI symbol
   for(unsigned i = 0 ; i < I; ++i)
      if((Gate_List[Input_VIdList[i]].symbol) != NULL)
         outfile << 'i' << i << ' ' << (*(Gate_List[Input_VIdList[i]].symbol)) << endl;
   //PO symbol
   for(unsigned i = 0 ; i < O; ++i)
      if((Gate_List[M+1+i].symbol) != NULL)
         outfile << 'o' << i << ' ' << (*(Gate_List[M+1+i].symbol)) << endl;

   //comment
   struct tm *Time; time_t _t; time(&_t); Time = localtime(&_t);
   outfile << 'c' << endl \
           << "            .aag file output by HUNG,GUO-LIANG" << endl \
           << "            Creation Date: " \
           << (1900 + Time->tm_year) << '/' << setfill('0') << right \
           << setw(2) << (1 + Time->tm_mon) << setw(1) << '/' \
           << setw(2) << Time->tm_mday      << setw(1) << ' ' \
           << setw(2) << Time->tm_hour      << setw(1) << ':' \
           << setw(2) << Time->tm_min       << setw(1) << ':' \
           << setw(2) << Time->tm_sec       << endl << setfill(' ') << left \
           << "----------------------------------------------------------" << endl \
           << "             [http://fmv.jku.at/aiger/FORMAT]             " << endl \
           << "The AIGER And-Inverter Graph (AIG) Format Version 20071012" << endl;
   return;
}

