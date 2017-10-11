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

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   string mess;
   stringstream ss1, ss2;
   ss1 << (this-(cirMgr->getGate(0)));
   ss2 << getLineNo();
   mess = this->getTypeStr() + "(" + ss1.str() + ")";
   if(this->symbol != NULL)mess += ("\"" + *(this->symbol) + "\"");
   mess += (", line " + ss2.str() );
   cout << "==================================================" << endl;
   cout << "= " << left << setw(46) <<       mess       << " =" << endl;
   cout << "==================================================" << endl;
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   unsigned _ref = cirMgr->use_DFS_refe();
   this->reportDFS(level, _ref, 1, true);
   return;
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   unsigned _ref = cirMgr->use_DFS_refe();
   this->reportDFS(level, _ref, 1, false);
   return;
}

void
CirGate::reportDFS( int level, unsigned _ref, unsigned depth, bool rep_in) const
{
   assert (level >= 0);
   cout << this->getTypeStr() << ' ' << (this-(cirMgr->getGate(0)));
   if(level < 1){ cout << endl; return; }
   if(this->DFS_ref == _ref){ cout << " (*)" << endl; return; }
   cout << endl;

   if(rep_in){
      for(unsigned i = 0 ; i < this->fanin__LIDList.size(); ++i){
         this->DFS_ref = _ref;//fanin(s) have been reported.
         for(unsigned j = 0 ; j < depth; ++j)cout << "  ";
         if((this->fanin__LIDList[i])%2) cout << '!';
         if(cirMgr->getGate((this->fanin__LIDList[i])/2))
            cirMgr->getGate((this->fanin__LIDList[i])/2)->reportDFS(level-1,_ref,depth+1,rep_in);
         else cout << "UNDEF " << (this->fanin__LIDList[i])/2 << endl;
      }
   }
   else{
      for(unsigned i = 0 ; i < this->fanout_LIDList.size(); ++i){
         this->DFS_ref = _ref;//fanout(s) have been reported.
         for(unsigned j = 0 ; j < depth; ++j)cout << "  ";
         if((this->fanout_LIDList[i])%2) cout << '!';
         cirMgr->getGate((this->fanout_LIDList[i])/2)->reportDFS(level-1,_ref,depth+1,rep_in);
      }
   }
   return;
}

