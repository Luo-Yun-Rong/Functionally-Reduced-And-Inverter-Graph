/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Yun-Rong Luo, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/
enum OptCase{
	FANIN_CONST1 = 1,
	FANIN_CONST0 = 2,
	IDENTICAL = 3,
	INVERTED = 4
};
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
void
CirMgr::sweep()
{
	for(int i=0;i<_gateList.size();i++){
		CirGate *g = _gateList[i];
		if(g!=NULL && g->getReach()== false && g->getType()!=PI_GATE 
			&& g->getType()!=CONST_GATE){
			if(g->getType()==AIG_GATE) A--;
			cout<<"Sweeping: "<<g->getTypeStr()<<"("<<i<<") removed..."<<endl;
			delete _gateList[i]; _gateList[i]=NULL;
		}
	}
	resetFloat(true);
	resetUnuse();
	resetFEC();
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...

void
CirMgr::optimize()
{
	CirGate  *g;
	OptCase optcase;
	size_t id0,ph0,id1,ph1; //fanin id & phase

	for(int i=0;i<_dfsList.size();++i){
		g = _dfsList[i];
		//optGate(g,true);
		if(g->FaninSize()==2){	
			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			id1 = g->getFaninGateID(1); ph1 = g->getFaninGatePhase(1);
			
			if(id0==id1){ 
				if(ph0==ph1) optcase = IDENTICAL;
				else optcase = INVERTED;
			}
			else if(id0==0 || id1==0){
				#define AnthrId (id0==0 ? id1 : id0)
				#define AnthrPh (id0==0 ? ph1 : ph0)
				#define ZeroPh (id0==0 ? ph0 : ph1)
				if(ZeroPh==0) optcase = FANIN_CONST0;
				else optcase = FANIN_CONST1;
			}
			else continue;

			cout<<"Simplifying: ";
			switch(optcase){
				case FANIN_CONST1:
					mergeGate(g,_gateList[AnthrId],AnthrPh);
					break;
				case IDENTICAL:
					mergeGate(g,_gateList[id0],ph0);
					break;
				case FANIN_CONST0:
				case INVERTED:
					mergeGate(g,_gateList[0],0);
					break;
				default:
					break;
			}
		}
	}
	resetFloat();
	resetDfs();
	resetUnuse();
	resetFEC();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::resetFloat(bool cirsw){
	_floatList.clear();
	if(cirsw){
		for(int i=0;i<_gateList.size();++i){
			CirGate  *g = _gateList[i];
			if(g!=NULL && g->getType()!=PO_GATE)
				g->clearFanout();
		}
	}
	for(int i=0;i<_gateList.size();i++){
		CirGate  *g = _gateList[i];
		if(g==NULL) continue;
		if(g->getType()==PO_GATE || g->getType()==AIG_GATE){
			for(int j=0;j<g->FaninSize();j++){
				size_t id = g->getFaninGateID(j);
				size_t phase = g->getFaninGatePhase(j);
				//floating case: b,c,d
				assert(id< _gateList.size());
				assert(_gateList[id]!=NULL);
				if(_gateList[id]-> getType() == UNDEF_GATE) {
					if(_floatList.empty()||_floatList.back()!=g)
						_floatList.push_back(g);//prevent repeat
				}
				//even undefined gate has to set fanout & fanin
				if(cirsw) _gateList[id] -> setFanout(g,phase);
			}
		}
	}
}
void
CirMgr::resetUnuse(){
	_unuseList.clear();
	for(int i=0;i<_gateList.size();i++){
		CirGate *g = _gateList[i];
		if(g == NULL) continue;
		else if(g -> getType() == PI_GATE || g-> getType()== AIG_GATE){
			if(g -> FanoutSize() == 0)
				_unuseList.push_back(g);
		}
	}
}
void
CirMgr::resetDfs(){
	//Clear and reset _dfsList
	for(int i=0;i<_gateList.size();++i){
		if(_gateList[i]!=NULL){
			_gateList[i]->setReach(false);
			_gateList[i]->setDfsNum(-1);
		}
	}
	_dfsList.clear(); 
	dfs();
}
//merge delGate to merGate, delete delGate
//if merGate is fanin of delGate, its fanout phase to delGate should be propagated
void
CirMgr::mergeGate(CirGate* delGate, CirGate *merGate,int propPhase){
	cout<<merGate->getID()<<" merging ";
	if(propPhase==1) cout<<"!";
	cout<<delGate->getID()<<"..."<<endl;

	size_t faninId;
	for(int i=0;i<delGate->FaninSize();++i){
		faninId = delGate->getFaninGateID(i);
		_gateList[faninId] -> removeFanout(delGate->getID());
	}
	size_t id; bool ph; //delGate's fanout id & ph
	for(int i=0;i<delGate->FanoutSize();++i){
		id = delGate->getFanoutGateID(i); 
		ph = delGate->getFanoutGatePhase(i);
		if(propPhase!=-1){
			_gateList[id]->replaceFanin(delGate->getID(),merGate,propPhase!=ph);
			merGate->setFanout(_gateList[id],propPhase!=ph);
		}
		else{
			_gateList[id]->replaceFanin(delGate->getID(),merGate,ph);
			merGate->setFanout(_gateList[id],ph);
		}
	}
	size_t gid = delGate->getID();
	if(delGate->getType()==AIG_GATE) A--;
	delete _gateList[gid]; _gateList[gid]=NULL;
}
