#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <sstream>

#include "engine.hpp"
#include "tools.hpp"

namespace KBX {

  inline bool readTo(std::istream &stream,char end){
    if(stream.tellg() > 0) stream.unget();
    char c;
    while(stream.good()){
      stream.get(c);
      if(c == end) break;
    }
    return (c==end);
  }

  inline bool readNext(std::istream &stream, std::stringstream& value){
    char c;
    value.clear();
    int ostack = 0;
    int lstack = 0;
    while(stream.good()){
      stream.get(c);
      if(ostack == 0 && lstack == 0){
	if(c == KBX::endList || c == KBX::endObj){
	  return false;
	}
	if(c==KBX::separator){
	  return true;
	}
      }
      switch(c){
      case KBX::endObj:
	ostack--;
	break;
      case KBX::beginObj:
	ostack++;
	break;
      case KBX::endList:
	lstack--;
	break;
      case KBX::beginList:
	lstack++;
	break;
      }
      value<<c;
    }
    return false;
  }
  inline bool readNextToken(std::istream &stream, std::string& key, std::stringstream& value){
    char c;
    key.clear();
    while(stream.good()){
      stream.get(c);
      if(c == ':') break;
      key+=c;
    }
    return readNext(stream,value);
  }

  std::ostream& operator<< (std::ostream &out, const Game& game){
    out << KBX::beginObj;
    out << "mode:" << game._mode << KBX::separator;
    out << "next:" << (KBX::PlayColor)(game._nextPlayer) << KBX::separator;
    out << "aiDepth:" << game._aiDepth << KBX::separator;
    out << "aiStrategy:" << game._strategy << KBX::separator;
    out << "dice:" << KBX::beginList;
    for (size_t i = 0; i < 18; i++) {
      out << game._dice[i];
      if(i!=17) out<< KBX::separator;
    }
    out << KBX::endList << KBX::separator;
    out << "history:";
    out << KBX::beginObj;
    out << "moves:" << KBX::beginList;
    for(auto it = game._moveStack.begin(); it != game._moveStack.end(); it++){
      out << (*it);
      if(!is_last(it,game._moveStack)) out<< KBX::separator;
    }
    out << KBX::endList << KBX::separator;
    out << "deaths:" << KBX::beginList;
    for(auto it = game._deathStack.begin(); it != game._deathStack.end(); it++){
      out << (*it);
      if(!is_last(it,game._deathStack)) out<< KBX::separator;
    }
    out << KBX::endList << KBX::separator;
    out << "movesPending:" << KBX::beginList;
    for(auto it = game._moveStackPending.begin(); it != game._moveStackPending.end(); it++){
      out << (*it);
      if(!is_last(it,game._moveStackPending)) out<< KBX::separator;
    }
    out << KBX::endList << KBX::separator;
    out << "deathsPending:" << KBX::beginList;
    for(auto it = game._deathStackPending.begin(); it != game._deathStackPending.end(); it++){
      out << (*it);
      if(!is_last(it,game._deathStackPending)) out<< KBX::separator;
    }
    out << KBX::endList;
    out << KBX::endObj;
    out << KBX::endObj;
    return out;
  }

  void peekAhead(std::istream& stream, int length){
    int pos = stream.tellg();
    char* s = (char*)calloc(length,sizeof(char));
    stream.read(s,length);
    std::cout << s << std::endl;
    free(s);
    stream.seekg(pos);
  }

  int readMoves(std::istream& stream,std::list<Move>& moves){
    int retval = 0;
    readTo(stream,KBX::beginList);
    while(stream.good()){
      readTo(stream,KBX::beginObj);
      Move m;
      stream >> m;
      if(m){
	moves.push_back(m);
	retval++;
      }
    }
    return retval;
  }

  int readDeaths(std::istream& stream,std::list<int>& deaths){
    int retval = 0;
    const int emptyval = -10;
    readTo(stream,KBX::beginList);
    while(stream.good()){
      int id = emptyval;
      stream >> id;
      if(id != emptyval){
	deaths.push_back(id);
	retval++;
      }
      readTo(stream,KBX::separator);
    }
    return retval;
  }

  std::istream& operator>> (std::istream & stream, Game& game){
    readTo(stream,KBX::beginObj);
    game.clearBoard();
    while(stream.good()){
      std::string key;
      std::stringstream value;
      bool next = readNextToken(stream,key,value);
      if(key.empty()) break;
      if(key == "mode"){
	size_t mode;
	value >> mode;
	game._mode = (KBX::PlayMode)(mode);
      } else if(key=="next"){
	int nextPlayer;
	value >> nextPlayer;
	game._nextPlayer = (KBX::PlayColor)(nextPlayer);
      } else if(key=="aiDepth"){
	value >> game._aiDepth;
      } else if(key=="aiStrategy"){
	value >> game._strategy;
      } else if(key=="dice"){
	for(size_t i=0; i<18; i++){
	  value >> game._dice[i];
	  if(!game._dice[i].gotKilled()){
	    game._fields[game._dice[i].x()][game._dice[i].y()] = i;
	  }
	}
      } else if(key=="history"){
	readTo(value,KBX::beginObj);
	while(value.good()){
	  std::string subkey;
	  std::stringstream val;
	  bool nextSub = readNextToken(value,subkey,val);
	  if(subkey == "moves"){
	    KBX::readMoves(val,game._moveStack);
	  } else if(subkey=="deaths"){
	    KBX::readDeaths(val,game._deathStack);
	  } else if(subkey=="movesPending"){
	    KBX::readMoves(val,game._moveStackPending);
	  } else if(subkey=="deathsPending"){
	    KBX::readDeaths(val,game._deathStackPending);
	  }
	  if(!nextSub) break;
	}
      } else {
	throw stringprintf("deserialization operator >>&Game: unknown key '%s'",key.c_str());
      }
      if(!next) break;
    }
    return stream;
  }

  std::ostream& operator<< (std::ostream &out, const DieState& die){
    out << KBX::beginObj;
    out << "x:"<<die._x << KBX::separator;
    out << "y:"<<die._y << KBX::separator;
    out << "col:"<<die._color << KBX::separator;
    out << "fS:"<<((int)(die._formerState)) << KBX::separator;
    out << "cS:"<<(int)(die._curState);
    out << KBX::endObj;
    return out;
  }

  std::istream& operator>> (std::istream &stream, DieState& d){
    readTo(stream,KBX::beginObj);
    std::string key;
    while(stream.good()){
      std::stringstream value;
      bool next = readNextToken(stream,key,value);
      if(key.empty()) break;
      if(key=="x")  value >> d._x;
      if(key=="y")  value >> d._y;
      if(key=="col"){
	int col = 0;
	value >> col;
	d._color = (KBX::PlayColor)col;
      }
      if(key=="fS") value >> d._formerState;
      if(key=="cS") value >> d._curState;
      if(!next) break;
    }
    return stream;
  }

  std::ostream& operator<< (std::ostream &out, const RelativeMove& move){
    out << KBX::beginObj;
    out << "dx:"<<move.dx << KBX::separator;
    out << "dy:"<<move.dy << KBX::separator;
    out << "fX:"<<move.firstX;
    out << KBX::endObj;
    return out;
  }

  std::istream& operator>> (std::istream &stream, RelativeMove& move){
    readTo(stream,KBX::beginObj);
    while(stream.good()){
      std::string key;
      std::stringstream value;
      bool next = readNextToken(stream,key,value);
      if(key.empty()) break;
      if(key=="dx") value >> move.dx;
      if(key=="dy") value >> move.dy;
      if(key=="fX") value >> move.firstX;
      if(!next) break;
    }
    return stream;
  }

  std::ostream& operator<< (std::ostream &out, const Move& move){
    out << KBX::beginObj;
    out << "idx:" << move.dieIndex << KBX::separator;
    out << "rel:" << move.rel;
    out << KBX::endObj;
    return out;
  }

  std::istream& operator>> (std::istream &stream, Move& move){
    readTo(stream,KBX::beginObj);
    while(stream.good()){
      std::string key;
      std::stringstream value;
      bool next = readNextToken(stream,key,value);
      if(key.empty()) break;
      if(key=="idx") value >> move.dieIndex;
      if(key=="rel"){
	value >> move.rel;
      }
      if(!next) break;
    }
    return stream;
  }

  std::ostream& operator<< (std::ostream &out, const Strategy& s){
    out << KBX::beginObj;
    out << "name:'" <<s.name << "'" << KBX::separator;
    out << "coeffDR:" <<s.coeffDiceRatio  << KBX::separator;
    out << "pat:" << s.patience;
    out << KBX::endObj;
    return out;
  }

  std::istream& operator>> (std::istream &stream, Strategy& s){
    readTo(stream,KBX::beginObj);
    while(stream.good()){
      std::string key;
      std::stringstream value;
      bool next = readNextToken(stream,key,value);
      if(key.empty()) break;
      if(key=="name") value >> s.name;
      if(key=="coeffDR") value >> s.coeffDiceRatio;
      if(key=="pat") value >> s.patience;
      if(!next) break;
    }
    return stream;
  }

} // end namespace KBX

