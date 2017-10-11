/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{
   cout << setw(15) << left << "MTReset: "
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;

   bool got_N = false, doArray = false;
   int getinput = 0;
   size_t numObjects = 0, arraySize = 0;
   for (size_t i = 0, n = options.size(); i < n; ++i){
      if(myStrNCmp("-Array", options[i], 2) == 0){
         if (doArray)
            return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         doArray = true;
         if ((++i) >= n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         if (!myStr2Int(options[i], getinput))
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         if (getinput <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         arraySize = (size_t)getinput;
      }
      else if(myStr2Int(options[i], getinput)) {
         if (got_N)
            return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         if (getinput <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         got_N = true;
         numObjects = (size_t)getinput;
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
   }
   if(!got_N)return CmdExec::errorOption(CMD_OPT_MISSING, "");
   try{
      if(doArray) mtest.newArrs( numObjects , arraySize );
      else        mtest.newObjs( numObjects );
   }
   catch(bad_alloc&){
      return CMD_EXEC_ERROR;
   }
   return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{
   cout << setw(15) << left << "MTNew: "
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;
   if (options.size() == 0)
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   bool doIndex = false, doRandom = false, doArray = false;
   int objId = 0, numRandId = 0, run_cerr = 0;
   for (size_t i = 0, n = options.size(); i < n; ++i){
      if (myStrNCmp("-Index", options[i], 2) == 0) {
         if (doIndex || doRandom)
            return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         doIndex = true;
         if ((++i) >= n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         if (!myStr2Int(options[i], objId))
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         if (objId < 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         run_cerr = i;
      }
      else if (myStrNCmp("-Random", options[i], 2) == 0){
         if (doIndex || doRandom)
            return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         doRandom = true;
         if ((++i) >= n)
            return CmdExec::errorOption(CMD_OPT_MISSING, options[i-1]);
         if (!myStr2Int(options[i], numRandId))
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         if (numRandId <= 0)
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         run_cerr = i-1;
      }
      else if (myStrNCmp("-Array", options[i], 2) == 0){
         if (doArray)
            return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         doArray = true;
      }
      else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
   }

   if(doArray){
   //deleteArray
      size_t Arr_Size = mtest.getArrListSize();
      if(doIndex){
         if (objId >= (signed)Arr_Size){
            cerr << "Size of array list (" << Arr_Size \
                 << ") is <= " << objId << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[run_cerr]);
         }
         mtest.deleteArr((size_t)objId);
      }
      else if(doRandom){
         if (Arr_Size == 0){
            cerr << "Size of array list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[run_cerr]);
         }
         for(int i = 0; i < numRandId; ++i){
            mtest.deleteArr((size_t)rnGen(Arr_Size));
         }
      }
      else return CmdExec::errorOption(CMD_OPT_MISSING, "");
   }
   else{
   //deleteObject
      size_t Obj_Size = mtest.getObjListSize();
      if(doIndex){
         if (objId >= (signed)Obj_Size){
            cerr << "Size of object list (" << Obj_Size \
                 << ") is <= " << objId << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[run_cerr]);
         }
         mtest.deleteObj((size_t)objId);
      }
      else if(doRandom){
         if (Obj_Size == 0){
            cerr << "Size of object list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[run_cerr]);
         }
         for(int i = 0; i < numRandId; ++i){
            mtest.deleteObj((size_t)rnGen(Obj_Size));
         }
      }
      else return CmdExec::errorOption(CMD_OPT_MISSING, "");
   }
   return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{
   cout << setw(15) << left << "MTDelete: "
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{
   cout << setw(15) << left << "MTPrint: "
        << "(memory test) print memory manager info" << endl;
}


