/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"

//#define PRINT_DEBUG_MESSAGE

using namespace std;

// TODO: define your own typedef or enum

class CirGate;
class CirMgr;
class SatSolver;

enum GateType
{
   UNDEF_GATE = 0x00,
   CONST_GATE = 0x01,
   PI_GATE    = 0x02,
   PO_GATE    = 0x04,
   AIG_GATE   = 0x08,

   TOT_GATE
};

#endif // CIR_DEF_H
