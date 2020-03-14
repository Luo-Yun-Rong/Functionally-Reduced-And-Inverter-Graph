/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Yun-Rong Luo, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <unordered_map>
using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

#define NUMSIG 5

void
CirMgr::strash()
{
	unordered_map<size_t,CirGate*>Strash;
	size_t f0,f1; size_t key;
	for(int i=0;i<_dfsList.size();++i){
		CirGate* gate = _dfsList[i];
		CirGate* merGate;
		if(gate->getType()==AIG_GATE){
			f0 = gate->getFaninGateID(0)*2 + gate->getFaninGatePhase(0);
			f1 = gate->getFaninGateID(1)*2 + gate->getFaninGatePhase(1);
			if(f0>=f1) key = (f0<<32)+f1; //<< precedence lower than +
			else key = (f1<<32)+f0; //<<precedence lower than +
		}
		else continue;
		
		if(Strash.find(key)!=Strash.end()){
			merGate = Strash[key];
			cout<<"Strashing: ";
			mergeGate(gate,merGate);
		}
		else Strash[key]=_dfsList[i];
	}
	resetFloat();
	resetDfs();
	resetUnuse();
	resetFEC();
}

void
CirMgr::fraig()
{
	SatSolver solver;
	solver.initialize();
	genProofModel(solver);
	
	int numSig=0;  
	bool result; 
	int id0,id1; bool ph0,ph1;
	bool finished=false;
	SortFEC(true);	
	while(true){
	//1.Simulation
		if(numSig==NUMSIG){ 
			resetDfs();
			resetFEC();
			assert(_sigList.size()==_piList.size());		
			for(int i=0;i<_sigList.size();i++){
				for(int j=numSig;j<64;j++){
					size_t bit = rnGen(2);
					while(bit==2) bit = rnGen(2);
					_sigList[i] = (_sigList[i]<<1)+ (size_t)(bit);
				}
			}
			for(int i=0;i<_piList.size();i++)
				_piList[i]->setSignal(_sigList[i]);
			simulate();
			if(_simLog!=NULL) writeSim(numSig);
			bool change = IdentifyFEC();
			SortFEC(true);
		}
	
	//2.Call SAT engine to prove FEC pair
		numSig=0; 
		_sigList.clear(); finished=false;
		for(int i=0;i<_fecGrps.size();i++){
			FECgroup *grp = _fecGrps[i];
			for(int j=0;j<grp->size();j++){
				for(int k=j+1;k<grp->size();k++){
					if(i==_fecGrps.size()-1 && j==grp->size()-2 
					   && k == grp->size()-1) 
						finished = true;
					
					id0 = grp->at(j)/2; ph0 = grp->at(j)%2;
					id1 = grp->at(k)/2; ph1 = grp->at(k)%2;
					if(_gateList[id0]!=NULL && _gateList[id1]!=NULL){
						result = ProvePair(solver,id0,ph0,id1,ph1);
						//UNSAT
						if(!result){
							cout<<"Fraig: ";
							mergeGate(_gateList[id1],_gateList[id0],ph0!=ph1); 
						}
						else{
							collectPattern(solver,numSig);
							numSig++;
							if(numSig==NUMSIG) break;
						}
					}
				}//end of for k
				if(numSig==NUMSIG) break;
			}//end of for j
			if(numSig==NUMSIG) break;
		}//end of for i, prove finished
		if( (numSig==NUMSIG) && !finished) continue;
		break;
	}//end of while

	resetFloat();
	resetDfs();
	resetUnuse();
	resetFEC();
	
	if(numSig>0){
		assert(_sigList.size()==_piList.size());
		for(int i=0;i<_piList.size();i++)
			_piList[i]->setSignal(_sigList[i]);
		simulate();
		if(_simLog!=NULL) writeSim(64);
		IdentifyFEC();
		SortFEC(false);
	}
}


/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::genProofModel(SatSolver& s){
	int id0,id1; bool ph0,ph1;
	Var v;

	// CONST_GATE
	v = s.newVar();
	_gateList[0]->setVar(v);
	s.addAigCNF(v,v,true,v,false);
	for(int i=0;i<_piList.size();i++){
		v = s.newVar();
		_piList[i]->setVar(v);
	}
	for(int i=0;i<_dfsList.size();i++){
		CirGate *g = _dfsList[i];
		if(g->getType()==AIG_GATE){
			v = s.newVar();
			g->setVar(v);
		}
		else if(g->getType()==PO_GATE){
			id0 = g->getFaninGateID(0);
			g->setVar(_gateList[id0]->getVar());
		}
		
		if(g->getType()==AIG_GATE){
			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			id1 = g->getFaninGateID(1); ph1 = g->getFaninGatePhase(1);
			s.addAigCNF(g->getVar(),_gateList[id0]->getVar(),ph0,
						_gateList[id1]->getVar(),ph1);
		}
	}
}
bool
CirMgr::ProvePair(SatSolver &solver,int id0,bool ph0,int id1,bool ph1){
	Var newV = solver.newVar();
	solver.addXorCNF(newV,_gateList[id0]->getVar(),ph0,
					_gateList[id1]->getVar(),ph1);
	
	//four types of FEC pair
	//solver.addXorCNF(vf, va, fa, vb, fb)
	//	 a, b ->  a!= b, (fa=0,fb=0), unsat means  a == b, merge( a, b,0)
	//	 a,!b ->  a!=!b, (fa=0,fb=1), unsat means  a ==!b, merge( a,!b,1)
	//	!a, b -> !a!= b, (fa=1,fb=0), unsat means !a == b, merge(!a, b,1)
	//	!a,!b -> !a!=!b, (fa=1,fb=1), unsat means !a ==!b, merge(!a,!b,0)
					
	solver.assumeRelease();
	solver.assumeProperty(newV,true);
	bool result = solver.assumpSolve();
	return result;
}
void
CirMgr::collectPattern(SatSolver &solver, int numSig){
	for(int i=0;i<_piList.size();i++){
		size_t a = solver.getValue(_piList[i]->getVar());
		assert(a==0 || a==1);
		if(numSig==0) _sigList.push_back(a);
		else _sigList[i]=(_sigList[i]<<1)+a;
	}
}

