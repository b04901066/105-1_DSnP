/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
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
static size_t lineNo = 0;  // in printint, lineNo needs to ++
static size_t colNo  = 0;  // in printing, colNo needs to ++
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
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
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
         if(!myStr2Size_t(token, M)){
            errMsg = "number of variables(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==2){
         if(!myStr2Size_t(token, I)){
            errMsg = "number of PIs(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==3){
         if(!myStr2Size_t(token, L)){
            errMsg = "number of latches(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==4){
         if(!myStr2Size_t(token, O)){
            errMsg = "number of POs(" + token + ")";
            return parseError(ILLEGAL_NUM);
         }
      }
      else if(ar==5){
         if(!myStr2Size_t(token, A)){
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
   Gate_List      = new CirGate[M+O+1];  //M(PI+AIG) PO const
   input_patterns = new queue<size_t>[I];
   sim_hash       = new HashSet<FECNode>(getHashSize(M+O+1));
   Gate_List[0].type = CONST_GATE;
   Gate_List[0].def_line = 0;

   //I <LID>
   for(size_t i = 0 ; i < I; ++i){
      size_t literal_ID;
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
      if(!myStr2Size_t(buf, literal_ID)){
         errMsg = "PI literal ID(" + buf + ")";
         return parseError(ILLEGAL_NUM);
      }
      errInt = literal_ID;
      //const
      if(literal_ID/2 == 0) return parseError(REDEF_CONST);
      //exceed MAX
      if(literal_ID/2 > M) return parseError(MAX_LIT_ID);
      //invert
      if(literal_ID&1){
         errMsg = "PI";
         return parseError(CANNOT_INVERTED);
      }
      //redefine
      if(Gate_List[literal_ID>>1].type){
         errGate = &(Gate_List[literal_ID>>1]);
         return parseError(REDEF_GATE);
      }

      //write Gate
      Gate_List[literal_ID>>1].type = PI_GATE;
      Gate_List[literal_ID>>1].def_line = ++lineNo;
      Input_VIdList.push_back(literal_ID>>1);
   }

   //L <LID> <LID>
   for(size_t i = 0 ; i < L; ++i){
      colNo = 0;
      if(!getline( file , buf )){
         errMsg = "Latche literal ID";
         return parseError(MISSING_NUM);
      }
      ++lineNo;
   }

   //O <LID>
   for(size_t i = 0 ; i < O; ++i){
      size_t literal_ID;
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
      if(!myStr2Size_t(buf, literal_ID)){
         errMsg = "PO literal ID(" + buf + ")";
         return parseError(ILLEGAL_NUM);
      }
      errInt = literal_ID;
      //exceed MAX
      if(literal_ID/2 > M)return parseError(MAX_LIT_ID);

      //write Gate
      Gate_List[M+1+i].type = PO_GATE;
      Gate_List[M+1+i].def_line = ++lineNo;
      Gate_List[M+1+i].fanin__LID_0 = literal_ID;
      Gate_List[literal_ID>>1].fanout_LIDList.push_back(2*(M+1+i)+(literal_ID&1));
   }

   //A <LHS> <RHS1> <RHS2>
   for(size_t i = 0 ; i < A; ++i){
      size_t literal_ID, LHS, RHS1, RHS2;
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
         if(!myStr2Size_t(token, literal_ID)){
            errMsg = "AIG literal ID(" + buf + ")";
            return parseError(ILLEGAL_NUM);
         }
         errInt = literal_ID;
         //const
         if(ar == 0 && literal_ID/2 == 0) return parseError(REDEF_CONST);
         //exceed MAX
         if(literal_ID/2 > M) return parseError(MAX_LIT_ID);
         //invert
         if(ar == 0 && literal_ID&1){
            errMsg = "AIG";
            return parseError(CANNOT_INVERTED);
         }
         //redefine
         if(ar == 0 && Gate_List[literal_ID>>1].type){
            errGate = &(Gate_List[literal_ID>>1]);
            return parseError(REDEF_GATE);
         }
              if(ar==0) LHS  = literal_ID;
         else if(ar==1) RHS1 = literal_ID;
         else if(ar==2) RHS2 = literal_ID;
         colNo += (token.length()+1);
      }
      //write Gate
      Gate_List[LHS>>1].type = AIG_GATE;
      Gate_List[LHS>>1].def_line = ++lineNo;
      Gate_List[LHS>>1].fanin__LID_0 = RHS1;
      Gate_List[RHS1>>1].fanout_LIDList.push_back(LHS+(RHS1&1));
      Gate_List[LHS>>1].fanin__LID_1 = RHS2;
      Gate_List[RHS2>>1].fanout_LIDList.push_back(LHS+(RHS2&1));
   }

   //Symbol [ilo]<pos> <string>
   while(getline( file , buf )){
      colNo = 0;
      char ilo;
      size_t pos;
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
      if(!myStr2Size_t(buf, pos)){
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
      for(size_t re = 0; re < name.length() ; ++colNo, ++re){
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
   ODGL();
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
void CirMgr::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics" << endl
        << "==================" << endl
        << "  PI   "  <<  right << setw(11) << I << endl
        << "  PO   "  <<  right << setw(11) << O << endl
        << "  AIG  "  <<  right << setw(11) << A << endl
        << "------------------" << endl
        << "  Total"  <<  right << setw(11) << (I+O+A) << endl;
   cout << left;
   return;
}

void CirMgr::printNetlist() const
{
   cout << endl;
   for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
      cout << '[' << i << "] ";
      this->print_Gate(Gate_List[DFS_GateList[i]]);
   }
   return;
}

void CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(size_t i = 0, s = Input_VIdList.size(); i < s; ++i)
      cout << ' ' << Input_VIdList[i];
   cout << endl;
   return;
}

void CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(size_t i = M+1 ; i < (M+O+1); ++i) cout << ' ' << i;
   cout << endl;
   return;
}

void CirMgr::printFloatGates() const
{
   bool f_found = false, nu_found = false;
   for(size_t i = 0 ; i < (M+O+1); ++i){
      if(Gate_List[i].type & (PO_GATE | AIG_GATE)){
         if(Gate_List[Gate_List[i].fanin__LID_0>>1].type == UNDEF_GATE){
            if(!f_found){
               cout << "Gates with floating fanin(s):";
               f_found = true;
            }
            cout << ' ' << i;//UNDEF
         }
         else if(Gate_List[i].type == AIG_GATE){
            if(Gate_List[Gate_List[i].fanin__LID_1>>1].type == UNDEF_GATE){
               if(!f_found){
                  cout << "Gates with floating fanin(s):";
                  f_found = true;
               }
               cout << ' ' << i;//UNDEF
            }
         }
      }
   }
   if(f_found)cout << endl;

   for(size_t i = 0 ; i < (M+1); ++i){
      if(Gate_List[i].type & (PI_GATE | AIG_GATE)){
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

void CirMgr::printFECPairs() const
{
   if(sim_flag == 0) return;
   vector<size_t>* FECGroups = new vector<size_t>[M+1];
   size_t* Leader = new size_t[M+1];
   // copy
   for(size_t i = 0 ; i <= M ; ++i)
      Leader[i] = (Gate_List[i].NEW_leader_LID>>1);
   // i -> leader -> leader
   for(size_t i = 0 ; i <= M ; ++i){
      if((Leader[Leader[i]]) > i){
          Leader[Leader[i]] = i;
                 Leader[i]  = i;
      }
      else if((Leader[Leader[i]]) < i){
         Leader[i] = Leader[Leader[i]];
      }
   }
   // collect group
   for(size_t i = 0 ; i <= M ; ++i){
      if((Leader[i] != i) && (Gate_List[i].type == AIG_GATE)){
         FECGroups[Leader[i]].push_back(i);
      }
   }
   // handle PI
   for(size_t i = 0 ; i < I ; ++i){
      if(FECGroups[Input_VIdList[i]].size() > 1){
         FECGroups[FECGroups[Input_VIdList[i]].front()].insert(\
         FECGroups[FECGroups[Input_VIdList[i]].front()].begin(),\
                (++FECGroups[Input_VIdList[i]].begin()),\
                   FECGroups[Input_VIdList[i]].end());
      }
      FECGroups[Input_VIdList[i]].clear();
   }
   // print
   size_t idx = 0;
   for(size_t i = 0 ; i <= M ; ++i){
      if(Gate_List[i].type & (CONST_GATE | AIG_GATE)){
         if(!FECGroups[i].empty()){
            cout << '[' << idx++ << "] " << i;
            for(size_t j = 0, n = FECGroups[i].size() ; j < n ; ++j){
               cout << ' ' << ( (Gate_List[i].sim_output == Gate_List[FECGroups[i][j]].sim_output) ? "":"!" ) << FECGroups[i][j];
            }
            cout << endl;
         }
      }
   }
   return;
}

void CirMgr::writeAag(ostream& outfile) const
{
   size_t temp_A = 0;
   for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
      if(Gate_List[DFS_GateList[i]].type == AIG_GATE) ++temp_A;
   }

   //header
   outfile <<  "aag "  << M << ' ' << I << ' '\
           << L << ' ' << O << ' ' << temp_A << endl;
   //PI
   for(size_t i = 0 ; i < I; ++i)
      outfile << (2*Input_VIdList[i]) << endl;
   //Latch
   for(size_t i = 0 ; i < L; ++i)
      outfile << endl;
   //PO
   for(size_t i = 0 ; i < O; ++i)
      outfile << (Gate_List[M+1+i].fanin__LID_0) << endl;
   //A
   for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
      if(Gate_List[DFS_GateList[i]].type == AIG_GATE){
         outfile << (2*DFS_GateList[i]) << ' '                       //<LHS>
                 << Gate_List[DFS_GateList[i]].fanin__LID_0 << ' '   //<RHS1>
                 << Gate_List[DFS_GateList[i]].fanin__LID_1 << endl; //<RHS2>
      }
   }

   //PI symbol
   for(size_t i = 0 ; i < I; ++i)
      if((Gate_List[Input_VIdList[i]].symbol) != NULL)
         outfile << 'i' << i << ' ' << (*(Gate_List[Input_VIdList[i]].symbol)) << endl;
   //PO symbol
   for(size_t i = 0 ; i < O; ++i)
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

void CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
   size_t temp_M = 0;
   vector<size_t> new_I;
   vector<size_t> new_A;
   ++DFS_refe;
   write_DFS_GateList(new_A, *g);

   //PI & fresh M
   for(size_t i = 0 ; i < I; ++i)
      if(Gate_List[Input_VIdList[i]].DFS_ref == DFS_refe){
         new_I.push_back(Input_VIdList[i]);
         if(temp_M < Input_VIdList[i])temp_M = Input_VIdList[i];
      }
   //fresh M
   for(size_t i = 0, n = new_A.size(); i < n; ++i){
      if(temp_M < new_A[i])temp_M = new_A[i];
      if(temp_M < Gate_List[new_A[i]].fanin__LID_0/2)temp_M = Gate_List[new_A[i]].fanin__LID_0/2;
      if(temp_M < Gate_List[new_A[i]].fanin__LID_1/2)temp_M = Gate_List[new_A[i]].fanin__LID_1/2;
   }
   //header
   outfile <<  "aag "  << temp_M << ' ' << new_I.size() << " 0 1 " << new_A.size() << endl;
   //PI
   for(size_t i = 0 , n = new_I.size() ; i < n; ++i)
      outfile << (2*new_I[i]) << endl;
   //Latch
   for(size_t i = 0 ; i < L; ++i)
      outfile << endl;
   //PO
   outfile << 2*(g-Gate_List) << endl;
   //A
   for(size_t i = 0, n = new_A.size() ; i < n; ++i){
      outfile << (2*new_A[i]) << ' '                       //<LHS>
              << Gate_List[new_A[i]].fanin__LID_0 << ' '   //<RHS1>
              << Gate_List[new_A[i]].fanin__LID_1 << endl; //<RHS2>
   }

   //PI symbol
   for(size_t i = 0 , n = new_I.size() ; i < n; ++i)
      if((Gate_List[new_I[i]].symbol) != NULL)
         outfile << 'i' << i << ' ' << (*(Gate_List[new_I[i]].symbol)) << endl;
   //PO symbol
   outfile << "o0 " << (g-Gate_List) << endl;

   //comment
   outfile << 'c' << endl
           << "            Write gate (" << (g-Gate_List) << ')' << endl;

   struct tm *Time; time_t _t; time(&_t); Time = localtime(&_t);
   outfile << "            .aag file output by HUNG,GUO-LIANG" << endl \
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

bool CirMgr::myStr2Size_t(const string& str, size_t& num)
{
   num = 0;
   bool valid = false;
   for(size_t i = 0 ; i < str.size() ; ++i){
      if(isdigit(str[i])) {
         num *= 10;
         num += unsigned(str[i] - '0');
         valid = true;
      }
      else return false;
   }
   return valid;
}

void CirMgr::print_Gate(CirGate& g) const
{
   //gate type & ID
   cout << left << setw(4) << g.getTypeStr() << setw(1) << ((&g)-Gate_List);
   if(g.type & (PO_GATE | AIG_GATE)){                                //fanin
      cout << ' ';
      if(Gate_List[g.fanin__LID_0>>1].type == UNDEF_GATE)cout << '*'; //UNDEF
      if(g.fanin__LID_0&1)cout << '!';                               //invert
      cout << g.fanin__LID_0/2;                                      //VID
   }
   if(g.type == AIG_GATE){                                           //fanin
      cout << ' ';
      if(Gate_List[g.fanin__LID_1>>1].type == UNDEF_GATE)cout << '*'; //UNDEF
      if(g.fanin__LID_1&1)cout << '!';                               //invert
      cout << g.fanin__LID_1/2;                                      //VID
   }
   if(g.symbol != NULL)cout << " (" << (*(g.symbol)) << ')';
   cout << endl;
   return;
}

void CirMgr::update_DFS_GateList(CirGate& g)
{
   if(g.DFS_ref == DFS_refe)return;
   g.DFS_ref = DFS_refe;
   if(g.type & (PO_GATE | AIG_GATE))
      update_DFS_GateList(Gate_List[g.fanin__LID_0>>1]);
   if(g.type == AIG_GATE)
      update_DFS_GateList(Gate_List[g.fanin__LID_1>>1]);
   if(g.type)DFS_GateList.push_back(((&g)-Gate_List));//post order
   return;
}

void CirMgr::write_DFS_GateList(vector<size_t>& A, CirGate& g) const
{
   if(g.DFS_ref == DFS_refe)return;
   g.DFS_ref = DFS_refe;
   if(g.type & (PO_GATE | AIG_GATE))
      write_DFS_GateList(A,Gate_List[g.fanin__LID_0>>1]);
   if(g.type == AIG_GATE){
      write_DFS_GateList(A,Gate_List[g.fanin__LID_1>>1]);
      A.push_back(((&g)-Gate_List));//post order
   }
   return;
}

void CirMgr::mark_DFS(CirGate& g) const
{
   if(g.DFS_ref == DFS_refe)return;
   g.DFS_ref = DFS_refe;
   if(g.type & (PO_GATE | AIG_GATE))mark_DFS(Gate_List[g.fanin__LID_0>>1]);
   if(g.type == AIG_GATE)mark_DFS(Gate_List[g.fanin__LID_1>>1]);
   return;
}

void CirMgr::removeAIG(CirGate& g)
{
   #ifdef PRINT_DEBUG_MESSAGE
   assert(g.type == AIG_GATE);
   bool check0 = false;
   bool check1 = false;
   cout << g.getTypeStr() << '(' << (size_t)((&g)-Gate_List) << ") removed..." << endl;
   #endif
   size_t g0 = g.fanin__LID_0>>1, g1 = g.fanin__LID_1>>1;
   for(size_t i = 0, n = Gate_List[g0].fanout_LIDList.size(); i < n ; ++i ){
      if((Gate_List[g0].fanout_LIDList[i]>>1) == (size_t)((&g)-Gate_List)){
         Gate_List[g0].fanout_LIDList.erase(Gate_List[g0].fanout_LIDList.begin()+i);
         #ifdef PRINT_DEBUG_MESSAGE
         check0 = true;
         #endif
         break;
      }
   }
   for(size_t i = 0, n = Gate_List[g1].fanout_LIDList.size(); i < n ; ++i ){
      if((Gate_List[g1].fanout_LIDList[i]>>1) == (size_t)((&g)-Gate_List)){
         Gate_List[g1].fanout_LIDList.erase(Gate_List[g1].fanout_LIDList.begin()+i);
         #ifdef PRINT_DEBUG_MESSAGE
         check1 = true;
         #endif
         break;
      }
   }
   #ifdef PRINT_DEBUG_MESSAGE
   assert(check0);
   assert(check1);
   #endif
   g.NEW_leader_LID = (((size_t)((&g)-Gate_List))<<1);
   g.type = UNDEF_GATE;
   --A;
   return;
}

/********************** LID(A),   VID(B),  B fanout would be empty *****/
void CirMgr::Merge2AIG(size_t A, size_t B)
{
   #ifdef PRINT_DEBUG_MESSAGE
   assert(Gate_List[B].type == AIG_GATE);
   cout << (A>>1) << " merging " << (A&1?"!":"") << B << "..." << endl;
   #endif
   for(size_t i = 0, n = Gate_List[B].fanout_LIDList.size(); i < n; ++i ){
      if((Gate_List[Gate_List[B].fanout_LIDList[i]>>1].fanin__LID_0>>1) == B){
          Gate_List[Gate_List[B].fanout_LIDList[i]>>1].fanin__LID_0 =\
               A ^ (Gate_List[B].fanout_LIDList[i]&1);
          Gate_List[A>>1].fanout_LIDList.push_back(Gate_List[B].fanout_LIDList[i] ^ (A&1));
      }
 else if((Gate_List[Gate_List[B].fanout_LIDList[i]>>1].fanin__LID_1>>1) == B){
          Gate_List[Gate_List[B].fanout_LIDList[i]>>1].fanin__LID_1 =\
               A ^ (Gate_List[B].fanout_LIDList[i]&1);
          Gate_List[A>>1].fanout_LIDList.push_back(Gate_List[B].fanout_LIDList[i] ^ (A&1));
      }
   }
   Gate_List[B].fanout_LIDList.clear();
   return;
}
