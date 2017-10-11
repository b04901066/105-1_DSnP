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
#include <queue>
#include <ctime>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"
#include "myHashSet.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr():  Gate_List(NULL)     ,
              input_patterns(NULL),
              sim_hash(NULL)      ,
              _simLog(NULL)       ,
              DFS_refe(0)         ,
              sim_flag(0){}
   ~CirMgr(){ if(Gate_List      != NULL) delete []Gate_List;
              if(input_patterns != NULL) delete []input_patterns;
              if(sim_hash       != NULL) delete sim_hash; }

   // Access functions
   // return NULL if "gid" corresponds to an undefined gate.
   CirGate* getGate(size_t gid) const {
      if(gid > (M+O)) return NULL;
      else return ( (Gate_List[gid].type == UNDEF_GATE) ? NULL : &(Gate_List[gid]) ); }

   CirGate* gate() const { return Gate_List; }
   const size_t get_M() const { return M; }
   const size_t get_sim_flag() const { return sim_flag;}

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   size_t use_DFS_refe(){ return ++DFS_refe; }

private:
   class StrashNode{
      public:
         StrashNode(){}
         StrashNode(size_t V): _VID(V) {}
         ~StrashNode(){}
         size_t operator () () const {
            return ( (cirMgr->gate()+(_VID))->fanin__LID_0 +\
                     (cirMgr->gate()+(_VID))->fanin__LID_1 ); }
         bool operator == (const StrashNode& n) const{
            return ((((cirMgr->gate()+(this->_VID))->fanin__LID_0 == (cirMgr->gate()+(n._VID))->fanin__LID_0) &&\
                     ((cirMgr->gate()+(this->_VID))->fanin__LID_1 == (cirMgr->gate()+(n._VID))->fanin__LID_1)) ||\
                    (((cirMgr->gate()+(this->_VID))->fanin__LID_0 == (cirMgr->gate()+(n._VID))->fanin__LID_1) &&\
                     ((cirMgr->gate()+(this->_VID))->fanin__LID_1 == (cirMgr->gate()+(n._VID))->fanin__LID_0))); }
         size_t getID() const { return _VID; }
      private:
         size_t  _VID;
   };
   class FECNode{
      public:
         FECNode(){}
         FECNode(size_t V): _VID(V) {}
         ~FECNode(){}
         size_t operator () () const {
            return ( ((cirMgr->gate()+(_VID))->sim_output)*(~(cirMgr->gate()+(_VID))->sim_output) + (cirMgr->gate()+(_VID))->OLD_leader_VID); }
         bool operator == (const FECNode& n) const{
            return (( (cirMgr->gate()+(this->_VID))->OLD_leader_VID ==   (cirMgr->gate()+(n._VID))->OLD_leader_VID )&&\
                    (((cirMgr->gate()+(this->_VID))->sim_output  ==   (cirMgr->gate()+(n._VID))->sim_output) ||\
                     ((cirMgr->gate()+(this->_VID))->sim_output  == ~((cirMgr->gate()+(n._VID))->sim_output)))); }
         size_t getID() const { return _VID; }
      private:
         size_t  _VID;
   };

   /*****************************
   * M: maximum variable index  *
   * I: number of inputs        *
   * L: number of latches       *
   * O: number of outputs       *
   * A: number of AND gates     *
   *****************************/
   size_t            M,  I,  L,  O,  A;
   CirGate*          Gate_List        ; // ALL GATES
   queue<size_t>*    input_patterns   ; // sim input
   HashSet<FECNode>* sim_hash         ; // VID
   ofstream*         _simLog          ;
   mutable size_t    DFS_refe         ; // global ref
   size_t            sim_flag         ; // 0x1 had sim ; 0x2 had brute sim
   vector<size_t>    Input_VIdList    ; // VID
   vector<size_t>    DFS_GateList     ; // might have PI,PO,AIG,CONST_GATE


   bool myStr2Size_t(const string&, size_t&);
   void print_Gate(CirGate&) const;
   void update_DFS_GateList(CirGate&);
   void write_DFS_GateList(vector<size_t>&, CirGate&) const;
   void mark_DFS(CirGate&) const;
   void removeAIG(CirGate&);
   void Merge2AIG(size_t, size_t); // LID(A) VID(B), B will disappear
   void run_Sim();                 // update sim_output
 //static bool comp_level(size_t i, size_t j){
 //   return ( (*(cirMgr->gate()+i)) < (*(cirMgr->gate()+j)) ); }
   void ODGL(){
      ++DFS_refe;
      DFS_GateList.clear();
      for(size_t i = 0 ; i < O; ++i)
         update_DFS_GateList(Gate_List[M+1+i]);
      return;
   }
};

#endif // CIR_MGR_H
