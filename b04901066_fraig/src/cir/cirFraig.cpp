/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <algorithm>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashSet.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions
void CirMgr::strash()
{
   HashSet<StrashNode> strash_hash(getHashSize(DFS_GateList.size()));

   for(size_t i = 0, n = DFS_GateList.size(); i < n; ){
      StrashNode temp(DFS_GateList[i]);
      if(Gate_List[DFS_GateList[i]].type != AIG_GATE){ ++i; }
      else if(!strash_hash.insert( temp )){
         strash_hash.query( temp );
         #ifdef PRINT_DEBUG_MESSAGE
         cout << "Strashing: ";
         #endif
         Merge2AIG(2*(temp.getID()),DFS_GateList[i]);
         removeAIG(Gate_List[DFS_GateList[i]]);
         DFS_GateList.erase(DFS_GateList.begin()+i);
         --n;
      }
      else{ ++i; }
   }
   return;
}

void CirMgr::fraig()
{
   size_t diff_num = 0;
   if(sim_flag & 2){
      for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
         CirGate& g = Gate_List[DFS_GateList[i]];
         if(( g.type == AIG_GATE ) && ( (g.NEW_leader_LID>>1) != DFS_GateList[i] )){
            Merge2AIG(g.NEW_leader_LID, DFS_GateList[i]);
            removeAIG(g);
         }
      }
   }
   else{
      /*(1) Create a SAT solver object (e.g. SatSolver s;)*/
      SatSolver solver;
      /*(2) Initialize it (by SatSolver::initialize())*/
      solver.initialize();
      /*(3) Generate the proof model from the circuit.*/
      /* CONST_GATE */
      Gate_List[0]._var = solver.newVar();
      solver.addConstCNF(Gate_List[0]._var);
      /*For each PI and AIG gate, assign a distinct "variable ID".*/
      for(size_t i = 1; i <= M; ++i){
         if(Gate_List[i].type == UNDEF_GATE)Gate_List[i]._var = Gate_List[0]._var;
         else Gate_List[i]._var = solver.newVar();
      }
      /*For each AIG gate, construct the CNF formula for it and add to the solver.*/
      for(size_t i = 1; i <= M; ++i){
         CirGate& g = Gate_List[i];
         if(g.type == AIG_GATE){
            /*addAndCNF(vf, va, fa, vb, fb)*/
            solver.addAigCNF(g._var, Gate_List[g.fanin__LID_0/2]._var, (g.fanin__LID_0%2),
                                     Gate_List[g.fanin__LID_1/2]._var, (g.fanin__LID_1%2));
         }
      }

      for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
         CirGate& g = Gate_List[DFS_GateList[i]];
         /*(4) Add the SAT target as "assumption" to the solver instance.*/
         if(( g.type == AIG_GATE ) && ( g.NEW_leader_LID/2 != DFS_GateList[i] )){
            Var newV = solver.newVar();
            solver.addXorCNF(newV, Gate_List[g.NEW_leader_LID/2]._var, (g.NEW_leader_LID%2), g._var, false);
            solver.assumeRelease();  // Clear assumptions
            /*assume Gate_List[g.NEW_leader_LID/2]._var
              and g._var are different.*/
            solver.assumeProperty(newV, true);
            /*the SAT target is satisfiable. == are different.
              You can get the input assignment by the
              function "int SatSolver::getValue(Var)".*/
            if(solver.assumpSolve()){
               if(diff_num == 0){
                  for(size_t j = 0 ; j < I ; ++j){
                     input_patterns[j].push(rnGen(INT_MAX));
                  }
               }
               for(size_t j = 0 ; j < I ; ++j){
                  input_patterns[j].front() <<= 1;
                  input_patterns[j].front() |= solver.getValue(Gate_List[Input_VIdList[j]]._var);
               }
               ++diff_num;
            }
            /*If the above step returns unsatisfiable,
              we can conclude the FEC pair must be equivalent
              and we can then merge them.*/
            else{
               #ifdef PRINT_DEBUG_MESSAGE
               cout << "Fraig: ";
               #endif
               Merge2AIG(g.NEW_leader_LID, DFS_GateList[i]);
               removeAIG(g);
            }
         }
         if( diff_num >> 2 ){
            #ifdef PRINT_DEBUG_MESSAGE
            cout << "running Sim..." << endl;
            #endif
            run_Sim();
            diff_num = 0;
         }
      }
   }
   /*******************************************************/
   ODGL();
   strash();
   return;
}
