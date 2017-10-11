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

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
   friend class CirMgr;

   CirGate(): type(UNDEF_GATE), def_line(0), DFS_ref(0), symbol(NULL) {}
   ~CirGate(){ if(symbol!=NULL)delete symbol; }
   //virtual ~CirGate() {}

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
   unsigned getLineNo() const { return def_line; }

   // Printing functions
   //virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin( int level) const;
   void reportFanout(int level) const;

private:
   GateType          type;
   unsigned          def_line;
   mutable unsigned  DFS_ref;
   string*           symbol;
   vector<unsigned>  fanin__LIDList;
   vector<unsigned>  fanout_LIDList;

   void reportDFS( int level, unsigned _ref, unsigned depth, bool rep_in) const;
};

#endif // CIR_GATE_H

