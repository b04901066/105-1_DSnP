/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"

#define DOFILE_STACK_SIZE 1024

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   if( _dofileStack.size() >= DOFILE_STACK_SIZE ){
      cerr << "Error: dofile stack overflow ("
           << _dofileStack.size() << ")" << endl;
      return false;
   }
   if(_dofile != 0)_dofileStack.push(_dofile);

   _dofile = new ifstream(dof.c_str());

   if(!(*_dofile).is_open()){
      _dofile = NULL;
      if( !_dofileStack.empty() ){
         _dofile = _dofileStack.top();
         _dofileStack.pop();
      }
      return false;
   }
   return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
   _dofile->close();
   delete _dofile;
   _dofile = NULL;
   if( !_dofileStack.empty() ){
      _dofile = _dofileStack.top();
      _dofileStack.pop();
   }
   return;
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...
   CmdMap::const_iterator iter = _cmdMap.begin();
   for(;iter != _cmdMap.end(); ++iter)
      iter->second->help();
   cout<<endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());

   // TODO...
   string str = _history.back();
   string first_word;
   assert(str[0] != 0 && str[0] != ' ');
   CmdExec* e = NULL;
   size_t tok_pos = myStrGetTok(str,first_word);
   e = getCmd(first_word);
   if(e == NULL){
      cerr << "Illegal command!! (" << first_word << ")" << endl;
      return NULL;
   }
   if( tok_pos < str.length()-1){
      string temp = str.substr( tok_pos );
      size_t not_space = temp.find_first_not_of(" ");
      option = temp.substr( not_space );
   }
   return e;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. [Before] Null cmd
//    cmd> $
//    -----------
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    -----------
//    [Before] partially matched (multiple matches)
//    cmd> h$aaa                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$aaa                // and then re-print the partial command
//
// 3. [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $
//    -----------
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$hahah
//    [After Tab]
//    cmd> heLp $hahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//
// 4. [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. [Before] Already matched
//    cmd> help asd$fg
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$fg
//
// 6. [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location
//
void
CmdParser::listCmd(const string& str)
{
   // TODO...
   size_t not_space = str.find_first_not_of(" ");
   CmdMap::iterator iter;

   //case 1. Null or Only space
   if(not_space == string::npos){
      iter = _cmdMap.begin();
      cout<<endl;
      for(size_t i = 0;iter != _cmdMap.end(); ++iter , ++i ){
         cout << setw(12) << left << ((iter->first) + (iter->second->getOptCmd()));
         if( i%5 == 4 )cout << endl;
      }
      this->reprintCmd();
      return;
   }

   string token;
   myStrGetTok( str , token , not_space );
   //Cursor NOT on the first word
   if( (unsigned)(_readBufPtr-_readBuf) > (token.length()+(unsigned)not_space)){
      //case 5. Already matched
      for(iter = _cmdMap.begin(); iter != _cmdMap.end(); ++iter ){
         string full_CmdStr = ((iter->first)+(iter->second->getOptCmd()));
         if( full_CmdStr.length() == token.length() ){
            if( myStrNCmp( full_CmdStr , token , token.length() ) == 0 ){
               cout << endl;
               iter->second->usage(cout);
               this->reprintCmd();return;
            }
         }
      }
      //case 6. Cursor NOT on the first word and NOT matched command
      mybeep();return;
   }
   else{
   //Cursor on the first word || ' 's behind
      vector<CmdMap::iterator>partially_match;
      for(iter = _cmdMap.begin(); iter != _cmdMap.end(); ++iter ){
         string full_CmdStr = ((iter->first)+(iter->second->getOptCmd()));
         if( full_CmdStr.length() >= token.length() ){
            if( myStrNCmp( full_CmdStr , token , token.length() ) == 0 ){
               partially_match.push_back(iter);
            }
         }
      }
      //case 2. partially matched (multiple matches)
      if(partially_match.size() >  1){
         cout<<endl;
         size_t i = 0;
         for(; i < partially_match.size() ; ++i ){
            cout << setw(12) << left << ((partially_match[i]->first)\
                 + (partially_match[i]->second->getOptCmd()));
            if( (i%5 == 4) && (i != partially_match.size()-1) )cout << endl;
         }
         this->reprintCmd();
         return;
      }
      //case 3. partially matched(single match)
      if(partially_match.size() == 1){
         string full_CmdStr = (partially_match[0]->first) +\
               (partially_match[0]->second->getOptCmd());
         string Sub_Str2Insert = full_CmdStr.substr(token.length());
         Sub_Str2Insert += " ";
         for(size_t i = 0 ; i < Sub_Str2Insert.length() ; ++i ){
            insertChar(Sub_Str2Insert[i]);
         }
         return;
      }
      //case 4. partially No match
      if(partially_match.size() == 0){
         mybeep();return;
      }
   }
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   //CmdExec* e = 0;
   // TODO...
   CmdMap::iterator iter;
   for(iter = _cmdMap.begin(); iter != _cmdMap.end(); ++iter ){
      if(cmd.length()>=(iter->first).length()){
         if((myStrNCmp( ((iter->first) + (iter->second->getOptCmd())) , \
             cmd , ((iter->first).length()))) == 0 )
               return (iter->second);
      }
   }
   return NULL;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

