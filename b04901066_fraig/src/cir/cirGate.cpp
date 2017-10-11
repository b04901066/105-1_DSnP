/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void CirGate::reportGate() const
{
   string mess, mess2;
   stringstream ss1, ss2;
   ss1 << (this-(cirMgr->gate()));
   ss2 << getLineNo();
   mess = this->getTypeStr() + "(" + ss1.str() + ")";
   if(this->symbol != NULL)mess += ("\"" + *(this->symbol) + "\"");
   mess += (", line " + ss2.str() );
   cout << "==================================================" << endl;
   cout << "= " << left << setw(46) <<       mess       << " =" << endl;

   if((this->type & (AIG_GATE | CONST_GATE))&& cirMgr->get_sim_flag()){
      vector<size_t> FE;
      for(size_t i = 0 , n = cirMgr->get_M()+1 ; i < n ; ++i){
         if(((this->NEW_leader_LID>>1) == (((cirMgr->gate()+i)->NEW_leader_LID)>>1))&&\
            ((size_t)(this-(cirMgr->gate())) != i)&&\
            ((cirMgr->gate()+i)->type & (CONST_GATE|AIG_GATE)))
            FE.push_back(i);
      }
      for(size_t i = 0 , n = FE.size() ; i < n; ++i){
         stringstream gateID;
         gateID << FE[i];
         if( this->sim_output != (cirMgr->gate()+FE[i])->sim_output )mess2 += "!";
         mess2 += gateID.str() + " ";
      }
   }
   cout << "= FECs: "  << left << setw(41) << mess2 << '=' << endl;

   cout << "= Value: ";
   if(sizeof(sim_output)>4){
      for(int i = 63 ; i > 31 ; --i)
         cout << ((sim_output>>i)&1) << (i%4?"":"_");
      cout << "\b =" << endl << "=        ";
   }
   for(int i = 31 ; i > -1 ; --i)
      cout << ((sim_output>>i)&1) << (i%4?"":"_");
   cout << "\b =" << endl;
   #ifdef PRINT_DEBUG_MESSAGE
   cout << "  OLD "  << OLD_leader_VID
        << " NEW " << (NEW_leader_LID&1?"!":"") << (NEW_leader_LID>>1)
        << " fanin " << (fanin__LID_0&1?"!":"") << (fanin__LID_0>>1)
        << ' ' << (fanin__LID_1&1?"!":"") << (fanin__LID_1>>1) << endl;
   #endif
   cout << "==================================================" << endl;
}

void CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   size_t _ref = cirMgr->use_DFS_refe();
   this->reportDFS(level, _ref, 1, true);
   return;
}

void CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   size_t _ref = cirMgr->use_DFS_refe();
   this->reportDFS(level, _ref, 1, false);
   return;
}

void CirGate::reportDFS( int level, size_t _ref, size_t depth, bool rep_in) const
{
   assert (level >= 0);
   cout << this->getTypeStr() << ' ' << (this-(cirMgr->gate()));
   if(level < 1){ cout << endl; return; }
   if(this->DFS_ref == _ref){ cout << " (*)" << endl; return; }
   cout << endl;

   if(rep_in){
      if(this->type & (AIG_GATE|PO_GATE)){
         this->DFS_ref = _ref;//fanin(s) have been reported.
         for(size_t j = 0 ; j < depth; ++j)cout << "  ";
         if((this->fanin__LID_0)&1) cout << '!';
         ((cirMgr->gate()+((this->fanin__LID_0)>>1)))->reportDFS(level-1,_ref,depth+1,rep_in);
      }
      if(this->type == AIG_GATE){
         for(size_t j = 0 ; j < depth; ++j)cout << "  ";
         if((this->fanin__LID_1)&1) cout << '!';
         ((cirMgr->gate()+((this->fanin__LID_1)>>1)))->reportDFS(level-1,_ref,depth+1,rep_in);
      }
   }
   else{
      for(size_t i = 0 , n = this->fanout_LIDList.size(); i < n ; ++i){
         this->DFS_ref = _ref;//fanout(s) have been reported.
         for(size_t j = 0 ; j < depth; ++j)cout << "  ";
         if((this->fanout_LIDList[i])&1) cout << '!';
         ((cirMgr->gate()+((this->fanout_LIDList[i])>>1)))->reportDFS(level-1,_ref,depth+1,rep_in);
      }
   }
   return;
}
