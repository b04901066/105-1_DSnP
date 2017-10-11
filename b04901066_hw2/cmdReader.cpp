/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar getChar(istream&);

//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void
CmdParser::readCmd()
{
   if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else
      readCmdInt(cin);
}

void
CmdParser::readCmdInt(istream& istr)
{
   resetBufAndPrintPrompt();

   while (1) {
      ParseChar pch = getChar(istr);
      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : if(moveBufPtr(_readBufPtr-1)){deleteChar();} break;/* TODO */
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               resetBufAndPrintPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: moveBufPtr(_readBufPtr + 1); break;/* TODO */
         case ARROW_LEFT_KEY : moveBufPtr(_readBufPtr - 1); break;/* TODO */
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        :                                    /* TODO */
              insertChar(' ',(TAB_POSITION-((_readBufPtr-_readBuf)%TAB_POSITION))); break;
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY  : mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif
   }
   return;
}


// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool
CmdParser::moveBufPtr(char* const ptr)
{
   // TODO...
   if( ptr < _readBuf || ptr > _readBufEnd ){ mybeep(); return false;}  //step 1.
   while( ptr != _readBufPtr )                                          //step 2&3.
      cout << (( ptr > _readBufPtr )?(++_readBufPtr,"\033[C"):(--_readBufPtr,"\033[D"));
   return true;
}


// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if at _readBufEnd
// 3. Move the remaining string left for one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing character.
// 7. Call "moveBufPtr(...)" if needed.
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool
CmdParser::deleteChar()
{
   // TODO...
   if( _readBufPtr >= _readBufEnd ){ mybeep(); return false;} //step 2.
   assert( _readBufPtr >= _readBuf );                         //check
   cout << "\0337" << "\033[0K";                              //step 4. record & claer
   for(char* shift = _readBufPtr ; shift < _readBufEnd ; ++shift )
      cout << (*shift = *(shift+1));                          //step 3,4(print)&6.
   cout << "\0338";                                           //step 4. move cursor
   --_readBufEnd;                                             //step 5.
   return true;
}

// 1. Insert character 'ch' for "repeat" times at _readBufPtr
// 2. Move the remaining string right for "repeat" characters
// 3. The cursor should move right for "repeats" positions afterwards
// 4. Default value for "repeat" is 1. You should assert that (repeat >= 1).
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k', 3) ---
//
// cmd> This is kkkthe command
//                 ^
//
void
CmdParser::insertChar(char ch, int repeat)
{
   // TODO...
   assert(repeat >= 1);
   for(int i = 0; i < repeat ; ++i , ++_readBufPtr , ++_readBufEnd ){  //step 2.
      for(char* shift = (_readBufEnd+1); shift > _readBufPtr ; --shift ) *shift=*(shift-1);
      cout << (*_readBufPtr = ch);                                     //step 1.
   }
   cout << "\0337";
   for(char* pr = _readBufPtr ; pr < _readBufEnd ; ++pr )cout << *pr;
   cout << "\0338";                                                    //step 3.
   return;
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void
CmdParser::deleteLine()
{
   // TODO...
   cout << "\033[2K" << '\r'; //step 1.
   resetBufAndPrintPrompt();  //step 2&3.
}


// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If moving up... (i.e. index < _historyIdx)
// 1. If already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If index < 0, let index = 0.
//
// If moving down... (i.e. index > _historyIdx)
// 1. If already at bottom, beep and do nothing
// 2. If index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//
void
CmdParser::moveToHistory(int index)
{
   // TODO...
   if(( index < _historyIdx && _historyIdx <= 0) ||                     //step 1.
      ( index > _historyIdx && _historyIdx >= (signed)(_history.size()-1)) ){ mybeep(); return;}
   if( _historyIdx == (signed)_history.size() ){                        //If moving up   step 2.
      string str(_readBuf); _history.push_back(str); _tempCmdStored = true;}
   if( index < 0 ) index = 0;                                           //step 3.
   if( index >= (signed)_history.size() ) index = (_history.size()-1);  //If moving down step 2
   _historyIdx = index; retrieveHistory();
   if( _historyIdx == (signed)(_history.size()-1) ){                    //clear temp
      _history.pop_back(); _tempCmdStored = false;}
   return;
}


// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for future insertion
//
void
CmdParser::addHistory()
{
   // TODO...
   _readBufPtr = _readBuf;
   if( _readBuf < _readBufEnd ){                                                        //step 1.
      while( (*(_readBufEnd-1)) == ' ' && (_readBufPtr < _readBufEnd) ) *(--_readBufEnd) = 0;
      while(  (*_readBufPtr)    == ' ' && (_readBufPtr < _readBufEnd) ) *(_readBufPtr++) = 0; }
   if( _tempCmdStored ){ _history.pop_back(); _tempCmdStored = false;}                  //step 4.
   if( _readBufPtr < _readBufEnd ){ string str(_readBufPtr); _history.push_back(str);}  //step 2&3.
   _historyIdx = (signed)_history.size();                                               //step 5.
   return;
}


// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void
CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}
