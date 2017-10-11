/****************************************************************************
  FileName     [ cirOpt.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void CirMgr::sweep()
{
   ODGL();
   for(size_t i = 1 ; i <= M ; ++i)
      if(Gate_List[i].DFS_ref != DFS_refe && Gate_List[i].type == AIG_GATE){
         #ifdef PRINT_DEBUG_MESSAGE
         cout << "Sweeping: ";
         #endif
         removeAIG(Gate_List[i]);
      }
   return;
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void CirMgr::optimize()
{
   for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
      CirGate& g = Gate_List[DFS_GateList[i]];
      if(g.type != AIG_GATE) continue;

      /*(b) If one of the fanins of an AND gate is a constatnt 0,
        this AND gate can be removed and replaced by the constant 0
        (Note: If the list of fanouts of the "other fanin" becomes empty,
         and if this "other fanin" is an AIG or PI,
         it will be added to the list of unused gates)
         ***Update fanout_LIDlist***/
      /*(d) If one of the fanins of an AND gate is inverse to the other fanin,
        this AND gate can be removed and replaced by a constant 0.*/
      else if(  g.fanin__LID_0 == 0 || g.fanin__LID_1 == 0 ||\
         ((g.fanin__LID_0/2 == g.fanin__LID_1/2) && (g.fanin__LID_0 != g.fanin__LID_1))){
         #ifdef PRINT_DEBUG_MESSAGE
         cout << "Simplifying: ";
         #endif
         Merge2AIG(0, DFS_GateList[i]);
      }

      /*(a) If one of the fanins of an AND gate is a constant 1,
        this AND gate can be removed and replaced by the other fanin.*/
      else if( g.fanin__LID_0 == 1 ){
         #ifdef PRINT_DEBUG_MESSAGE
         cout << "Simplifying: ";
         #endif
         Merge2AIG(g.fanin__LID_1, DFS_GateList[i]);
      }

      /*(c) If both fanins of an AND gate are the same,
        this AND gate can be removed and replaced by its fanin*/
      else if( g.fanin__LID_1 == 1 || g.fanin__LID_0 == g.fanin__LID_1 ){
         #ifdef PRINT_DEBUG_MESSAGE
         cout << "Simplifying: ";
         #endif
         Merge2AIG(g.fanin__LID_0, DFS_GateList[i]);
      }
   }
   sweep();
   return;
}
