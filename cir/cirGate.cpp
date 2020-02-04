/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Yun-Rong Luo ]
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
/**************************************/
/*   Static varaibles and functions   */
/**************************************/
size_t CirGate::_globalRef=0;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
string
CirGate::getTypeStr() const
{
	switch(_type){
		case PI_GATE:
			return "PI";
		case PO_GATE:
			return "PO";
		case AIG_GATE:
			return "AIG";
		case CONST_GATE:
			return "CONST";
		case UNDEF_GATE:
			return "UNDEF";
		default:
			return "";
	}
}

void
CirGate::reportGate() const
{
	cout<<"=================================================="<<endl;
	cout.fill(' '); cout.width(49);

	string out;
	out = "= "+getTypeStr()+"("+to_string(_gateID)+")";
	if(_sym!=NULL)
		out+=("\""+(*_sym)+"\"");
	out+=(", line "+to_string(_lineNo));

	cout<<left<<out<<"="<<endl;
	cout<<"=================================================="<<endl;
	cout.setf(ios::right);
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   _globalRef++;
   cout<<getTypeStr()<<" "<<_gateID<<endl;
   recurFanin(level,0);
}

void
CirGate::recurFanin(int level,int space) const{
   if(level==0 || _faninList==NULL) return;

   for(int i=0;i<_faninList->size();i++){
	   cout<<string(space,' '); 
	   //string(n,c): "Fills the string with n consecutive copies of character c.
	   cout<<"  ";
	   if(_faninList -> at(i) -> isInv())
		   cout<<"!";
	   
	   CirGate *g =  _faninList -> at(i) -> gate();
	   if(g->_ref == _globalRef && level>1 && g ->_faninList!=NULL)
		   cout<<g->getTypeStr()<<" "<<g->getID()<<" (*)"<<endl;
	   else{
		   if(level >1) g->_ref = _globalRef;
		   cout<<g->getTypeStr()<<" "<<g->getID()<<endl;
		   g -> recurFanin(level-1,space+2);
	   }
   }
}
void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   _globalRef++;
   cout<<getTypeStr()<<" "<<_gateID<<endl;
   recurFanout(level,0);
}
void
CirGate::recurFanout(int level,int space)const{
   if(level==0||_fanoutList==NULL) return;

   for(int i=0;i<_fanoutList ->size();i++){
	   cout<<string(space,' ');
	   cout<<"  ";
	   if(_fanoutList->at(i)->isInv())
		   cout<<"!";
	   
	   CirGate *g = _fanoutList -> at(i)->gate();
	   if(g->_ref == _globalRef && level>1 && g ->_fanoutList!=NULL)
		   cout<<g->getTypeStr()<<" "<<g->getID()<<" (*)"<<endl;
	   else{
		   if(level>1) g->_ref = _globalRef;
		   cout<<g->getTypeStr()<<" "<<g->getID()<<endl;
		   g -> recurFanout(level-1,space+2);
	   }
   }   

}
// Fanin related
void
CirGate::setFanin(size_t var){
	CirGateV *g = new CirGateV;
	size_t *tmp = (size_t*)((void*)g);
	*tmp = var;

	if(_faninList == NULL)
		_faninList = new vector<CirGateV*>;
	_faninList-> push_back(g);
}
void
CirGate::setFanin(CirGate* g,size_t phase,int i){
	assert(_faninList!= NULL && i< _faninList -> size());
	CirGateV *gv = new CirGateV(g,phase);
	_faninList -> at(i) = gv;
}
//replace one of the fanin whose id=orgin to (g,phase)
//return number of replacement
void
CirGate::replaceFanin(size_t orgin, CirGate* g,size_t phase){
	for(int i=0;i<FaninSize();i++){
		if(getFaninGateID(i)==orgin){
			CirGateV *gv = new CirGateV(g,phase);
			delete _faninList->at(i);
			_faninList->at(i)=gv;
		}
	}
}
size_t
CirGate::getFanin(int i){
	CirGateV* g = _faninList -> at(i);
	size_t *tmp = (size_t*)((void*)g);
	return *tmp;
}
//Fanout related
void
CirGate::setFanout(CirGate* g, size_t phase){
	if(_fanoutList == NULL)
		_fanoutList = new vector<CirGateV*>;
	CirGateV *gv = new CirGateV(g,phase);
	_fanoutList -> push_back(gv);
}
//remove all of the fanout=id
void 
CirGate::removeFanout(size_t id){
	for(int i=0;i<FanoutSize();++i){
		if(getFanoutGateID(i)==id){
			_fanoutList->erase(_fanoutList->begin()+i);
			i--; //restart from i
		}
	}
}

//dfs related
void 
CirGate::dfs(vector<CirGate*> &_dfsList){
	if(_faninList == NULL) { 
		_dfsList.push_back(this);
		return;
	}
	for(int i=0;i<_faninList->size();i++){
		CirGate *g = _faninList->at(i)->gate();
		if(g->getType()==UNDEF_GATE){ g->setReach(true); continue;}
		if(g->_ref != _globalRef){
			g->_ref = _globalRef;
			g->dfs(_dfsList);
		}
	}
	this->setReach(true);
	_dfsList.push_back(this);
}



