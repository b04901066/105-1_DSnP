/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(): Gate_List(NULL), DFS_refe(0){}
   ~CirMgr(){ if(Gate_List!=NULL)delete []Gate_List; }

   // Access functions
   // return NULL if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
      return ( (Gate_List[gid].type == UNDEF_GATE) ? NULL : &(Gate_List[gid]) );
   }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;

   unsigned use_DFS_refe(){ return ++DFS_refe; }

private:
   unsigned          M,  // maximum variable index
                     I,  // number of inputs
                     L,  // number of latches
                     O,  // number of outputs
                     A;  // number of AND gates

   CirGate*          Gate_List;
   mutable unsigned  DFS_refe;
   vector<unsigned>  Input_VIdList;
   vector<unsigned>  AND_VIdList;

   bool myStr2Unsigned(const string& str, unsigned& num){
      num = 0;
      size_t i = 0;
      bool valid = false;
      for (; i < str.size(); ++i) {
         if (isdigit(str[i])) {
            num *= 10;
            num += int(str[i] - '0');
            valid = true;
         }
         else return false;
      }
      return valid;
   }

   void print_idx(bool reset = false) const {
      static unsigned idx;
      if(reset)idx = 0;
      else cout << '[' << idx++ << "] ";
   }

   void print_DFS(CirGate& g ,unsigned _ref) const {
      if( g.DFS_ref == _ref )return;
      g.DFS_ref = _ref;
      if(g.type == UNDEF_GATE)return;
      for(unsigned i = 0 ; i < g.fanin__LIDList.size(); ++i){
         print_DFS(Gate_List[g.fanin__LIDList[i]/2], _ref);
      }
      cout << endl;
      print_idx();
      //gate type & ID
      cout << left << setw(4) << g.getTypeStr() << setw(1) << ((&g)-Gate_List);
      for(unsigned i = 0 ; i < g.fanin__LIDList.size(); ++i){                //fanin
         cout << ' ';
         if(Gate_List[g.fanin__LIDList[i]/2].type == UNDEF_GATE)cout << '*'; //UNDEF
         if(g.fanin__LIDList[i]%2)cout << '!';                               //invert
         cout << g.fanin__LIDList[i]/2;                                      //VID
      }
      if(g.symbol != NULL)cout << " (" << (*(g.symbol)) << ')';
      return;
   }

   //counting == true: count  _A
   //counting == false:output _A
   void write_DFS_A(ostream& os, CirGate& g, unsigned& _A, unsigned _ref, bool counting) const {
      if( g.DFS_ref == _ref )return;
      g.DFS_ref = _ref;
      if(g.type == UNDEF_GATE)return;
      for(unsigned i = 0 ; i < g.fanin__LIDList.size(); ++i){
         write_DFS_A(os, Gate_List[g.fanin__LIDList[i]/2], _A, _ref, counting);
      }
      if(g.type == AIG_GATE){
         if(counting){ ++_A; }
         else{
            //A <LHS> <RHS1> <RHS2>
            os << (2*((&g)-Gate_List));
            for(unsigned i = 0 ; i < g.fanin__LIDList.size(); ++i){  //fanin
               os << ' ' << g.fanin__LIDList[i];
            }
            os << endl;
         }
      }
      return;
   }
};

#endif // CIR_MGR_H

