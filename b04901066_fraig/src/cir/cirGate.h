/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <iostream>
#include <string>
#include <vector>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   friend class CirMgr;
   CirGate(): type(UNDEF_GATE)      ,
              _var(var_Undef)       ,
              sim_output(0)         ,
              OLD_leader_VID(0)     ,
              NEW_leader_LID(0)     ,
              fanin__LID_0(UINT_MAX),
              fanin__LID_1(UINT_MAX),
            //fanin_level(0)        ,
              def_line(0)           ,
              DFS_ref(0)            ,
              symbol(NULL){}
   ~CirGate(){ if(symbol != NULL) delete symbol; }

   // Basic access methods
   string getTypeStr() const {
      switch(type){
         case UNDEF_GATE:return "UNDEF";
         case PI_GATE   :return "PI";
         case PO_GATE   :return "PO";
         case AIG_GATE  :return "AIG";
         case CONST_GATE:return "CONST";
         default        :return "UNDEF";
      }
   }
   size_t getLineNo() const { return def_line; }
   bool isAig() const { return ( type == AIG_GATE ); }
 //bool operator < (CirGate& j){ return (this->fanin_level < j.fanin_level);}

   // Printing functions
   void reportGate() const;
   void reportFanin( int level) const;
   void reportFanout(int level) const;

private:
   GateType          type          ;
   Var               _var          ; // SAT Var
   size_t            sim_output    ; // output
   size_t            OLD_leader_VID; // VID
   size_t            NEW_leader_LID; // LID
   size_t            fanin__LID_0  ; // LID
   size_t            fanin__LID_1  ; // LID
 //size_t            fanin_level   ;
   size_t            def_line      ; // in org file
   mutable size_t    DFS_ref       ; // mark
   string*           symbol        ;
   vector<size_t>    fanout_LIDList; // LID

   void reportDFS( int level, size_t _ref, size_t depth, bool rep_in) const;
};

#endif // CIR_GATE_H
