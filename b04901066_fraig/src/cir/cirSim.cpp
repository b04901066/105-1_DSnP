/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>
#include <vector>
#include <ctime>
#include "cirMgr.h"
#include "cirGate.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randomSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions
void CirMgr::randomSim()
{
   size_t  kai = 0;
   //brute force
   if( I < 16 ){
      size_t ones = 0; --ones;
      if(sizeof(size_t) > 4){
         kai = pow(2, ((I>6)?(I-6):0));
         for(size_t i = 0 ; i < kai ; ++i ){
            for( size_t j = 0 ; j < I ; ++j ){
               if(j == 0)input_patterns[0].push(0xAAAAAAAAAAAAAAAA);
               if(j == 1)input_patterns[1].push(0xCCCCCCCCCCCCCCCC);
               if(j == 2)input_patterns[2].push(0xF0F0F0F0F0F0F0F0);
               if(j == 3)input_patterns[3].push(0xFF00FF00FF00FF00);
               if(j == 4)input_patterns[4].push(0xFFFF0000FFFF0000);
               if(j == 5)input_patterns[5].push(0xFFFFFFFF00000000);
               if(j >  5)input_patterns[j].push(ones*((i>>(j-6))&1));
            }
         }
      }
      else{
         kai = pow(2,((I>5)?(I-5):0));
         for(size_t i = 0 ; i < kai ; ++i ){
            for(size_t j = 0 ; j < I ; ++j){
               if(j == 0)input_patterns[0].push(0xAAAAAAAA);
               if(j == 1)input_patterns[1].push(0xCCCCCCCC);
               if(j == 2)input_patterns[2].push(0xF0F0F0F0);
               if(j == 3)input_patterns[3].push(0xFF00FF00);
               if(j == 4)input_patterns[4].push(0xFFFF0000);
               if(j >  4)input_patterns[j].push(ones*((i>>(j-5))&1));
            }
         }
      }
      sim_flag |= 2;
      for(size_t r = 0 ; r < kai ; ++r) run_Sim();
      cout << (8*sizeof(size_t)*kai) << " patterns simulated." << endl;
      return;
   }
   else{
      size_t r = 0;
      clock_t t;
      t = clock();
      kai = (1024*(1+log10((double)DFS_GateList.size())));
      for( ; r < kai ; ++r){
         if((clock() - t) > CLOCKS_PER_SEC*5)break;
         run_Sim();
      }
      cout << (8*sizeof(size_t)*r) << " patterns simulated." << endl;
      return;
   }
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   size_t   pattern_num = 0, pattern_num_O = 0;
   string   load_pattern;

   //clear
   while(!input_patterns[0].empty()){
      for(size_t i = 0 ; i < I ; ++i)
         input_patterns[i].pop();
   }
   //read patterns
   while(!patternFile.eof()){
      for(size_t i = 0 , bits = 8*sizeof(size_t) ; i < bits; ++i){
         patternFile >> load_pattern;
         if(patternFile.eof())break;
         if(load_pattern.size() != I){
            cerr << "Error: Pattern(" << load_pattern << ") length("
                 << load_pattern.size()
                 << ") does not match the number of inputs("
                 << I << ") in a circuit!!" << endl
                 << "0 patterns simulated." << endl;
            //clear
            while(!input_patterns[0].empty()){
               for(size_t j = 0 ; j < I ; ++j)
                  input_patterns[j].pop();
            }
            return;
         }
         if(i == 0)
            for(size_t j = 0 ; j < I ; ++j)
               input_patterns[j].push((size_t)0);
         for(size_t j = 0 ; j < I ; ++j){
                 if(load_pattern[j] == '0'){;}
            else if(load_pattern[j] == '1'){
               input_patterns[j].back() |= (((size_t)1)<<i); }
            else{ 
               cerr << "Error: Pattern(" << load_pattern
                    << ") contains a non-0/1 character(‘"
                    << load_pattern[j] << "’)." << endl
                    << "0 patterns simulated." << endl;
               //clear
               while(!input_patterns[0].empty()){
                  for(size_t k = 0 ; k < I ; ++k)
                     input_patterns[k].pop();
               }
               return;
            }
         }
         ++pattern_num;
      }
   }
   patternFile.close();

   //output file
   pattern_num_O = pattern_num;
   while(!input_patterns[0].empty()){
      run_Sim();
      if(_simLog){
         for(size_t i = 0 , bits = 8*sizeof(size_t) ; i < bits && pattern_num_O > 0 ; ++i){
            for(size_t j = 0 ; j < I; ++j)
               (*_simLog) << ((Gate_List[Input_VIdList[j]].sim_output>>i)&1);
            (*_simLog) << ' ';
            for(size_t j = 0 ; j < O; ++j)
               (*_simLog) << ((Gate_List[M+1+j].sim_output>>i)&1);
            (*_simLog) << endl;
            --pattern_num_O;
         }
      }
   }

   cout << pattern_num << " patterns simulated." << endl;
   return;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::run_Sim()
{
   //Load Input
   if(input_patterns[0].empty()){
      for(size_t i = 0 ; i < I ; ++i){
         Gate_List[Input_VIdList[i]].sim_output = rnGen(INT_MAX);
         if(sizeof(size_t)>4){
            Gate_List[Input_VIdList[i]].sim_output <<= 32;
            Gate_List[Input_VIdList[i]].sim_output += (~rnGen(INT_MAX));
         }
      }
   }
   else{
      for(size_t i = 0 ; i < I ; ++i){
         Gate_List[Input_VIdList[i]].sim_output = input_patterns[i].front();
         input_patterns[i].pop();
      }
   }

   //Start sim
   for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
      CirGate& g = Gate_List[DFS_GateList[i]];
      if(g.type == AIG_GATE){
         g.sim_output = (((g.fanin__LID_0&1)        ?\
         ~(Gate_List[g.fanin__LID_0>>1].sim_output)  :\
          (Gate_List[g.fanin__LID_0>>1].sim_output)) &\
          (         (g.fanin__LID_1&1)              ?\
         ~(Gate_List[g.fanin__LID_1>>1].sim_output)  :\
          (Gate_List[g.fanin__LID_1>>1].sim_output)));}
      else if(g.type == PO_GATE){
         g.sim_output = ((g.fanin__LID_0&1)         ?\
         ~(Gate_List[g.fanin__LID_0>>1].sim_output)  :\
          (Gate_List[g.fanin__LID_0>>1].sim_output)) ;}
   }

   sim_hash->clear();
   //const PI
   sim_hash->insert( FECNode(0) );
   for(size_t i = 0; i < I; ++i){
      FECNode temp(Input_VIdList[i]);
      if(sim_hash->insert( temp )){
         Gate_List[Input_VIdList[i]].NEW_leader_LID = 2*Input_VIdList[i];
      }
      else{
         sim_hash->query( temp );
         if( Gate_List[temp.getID()].sim_output == Gate_List[Input_VIdList[i]].sim_output )
            Gate_List[Input_VIdList[i]].NEW_leader_LID = 2*temp.getID();
         else if( Gate_List[temp.getID()].sim_output == ~(Gate_List[Input_VIdList[i]].sim_output) )
            Gate_List[Input_VIdList[i]].NEW_leader_LID = 2*temp.getID()+1;
         #ifdef PRINT_DEBUG_MESSAGE
         else assert(false);
         #endif
      }
   }
   //aig
   for(size_t i = 0, n = DFS_GateList.size(); i < n; ++i){
      if(Gate_List[DFS_GateList[i]].type == AIG_GATE){
         FECNode temp(DFS_GateList[i]);
         if(sim_hash->insert( temp )){
            Gate_List[DFS_GateList[i]].NEW_leader_LID = 2*DFS_GateList[i];
         }
         else{
            sim_hash->query( temp );
            if( Gate_List[temp.getID()].sim_output == Gate_List[DFS_GateList[i]].sim_output )
               Gate_List[DFS_GateList[i]].NEW_leader_LID = 2*temp.getID();
            else if( Gate_List[temp.getID()].sim_output == ~(Gate_List[DFS_GateList[i]].sim_output) )
               Gate_List[DFS_GateList[i]].NEW_leader_LID = 2*temp.getID()+1;
            #ifdef PRINT_DEBUG_MESSAGE
            else assert(false);
            #endif
         }
      }
   }
   for(size_t i = 0 ; i <= M ; ++i)
      Gate_List[i].OLD_leader_VID = (Gate_List[i].NEW_leader_LID>>1);

   sim_flag |= 1;
   return;
}
