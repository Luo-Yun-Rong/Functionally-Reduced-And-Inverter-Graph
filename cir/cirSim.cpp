/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Yun-Rong Luo, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <unordered_map>
#include <queue>
using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/


class FecListSort{
public:
	bool operator()(const FECgroup *fgp0, const FECgroup *fgp1)const{
		int id0 = fgp0->at(0)/2; 
		int id1 = fgp1->at(0)/2;
		return 	fgp0->size() < fgp1->size() || id0<id1;
	}
};

class FecGrpSort{
public:
	bool operator()(const int g0, const int g1)const{
		int id0 = g0/2; 
		int id1 = g1/2;		
		return cirMgr-> _gateList[id0]->getDfsNum() < 
			   cirMgr-> _gateList[id1]->getDfsNum();
	}				
};


/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
	CreateFirstFEC();
	int num=0, noNew=0; 
	while(true){
		randSig();
		assert(_sigList.size()==_piList.size());
		for(int i=0;i<_piList.size();i++)
			_piList[i]->setSignal(_sigList[i]);
		
		num+=64;
		simulate(); //simulate 64 pattern
		if(_simLog!=NULL) writeSim(64); //write 64 pattern
		if(!IdentifyFEC()) noNew++; 
		if(noNew>_piList.size()*2)break;
	}
	cout<<num<<" patterns simulated."<<endl;
	SortFEC(false);
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	CreateFirstFEC();
	int num=readSig(patternFile);
	cout<<num<<" patterns simulated."<<endl;

	for(int i=0;i<_sigList.size();i++){
		_piList[i%_piList.size()] -> setSignal(_sigList[i]);
		if(i%_piList.size()==_piList.size()-1){
			simulate(); //simulate 64 pattern
			if(_simLog!=NULL && num>=64) writeSim(64); //write 64 pattern
			else if(_simLog!=NULL && num<64) writeSim(num);
			IdentifyFEC();
			num-=64;
		}
	}
	SortFEC(false);
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
//generate 64 signals
void
CirMgr::randSig()
{
	_sigList.clear();
	size_t randint;
	for(int i=0;i<2;i++){
		for(int j=0;j<_piList.size();j++){
			//1st gernerated pattern will be leftmost bit of signal 
			randint = rnGen(INT_MAX);

			if(i==0) _sigList.push_back(randint);
			else
				_sigList[j] = (_sigList[j]<<32) + randint;
		}
	}	
}
int
CirMgr::readSig(ifstream& fin)
{

	_sigList.clear();
	int ofst=0; bool finish=false; int num=0;
	string pat;
	while(true){
		for(int i=0;i<64;i++){
			if(fin>>pat){
				if(pat.length() !=_piList.size()){
					cerr<<"Error: Pattern("<<pat<<") length("<<pat.length();
					cerr<<") does not match the number of inputs("<<_piList.size();
					cerr<<") in a circuit!!"<<endl;
					_fecGrps.clear();
					return 0;
				}
				for(int j=0;j<pat.length();j++){
					if(pat[j]!='0' && pat[j]!='1'){
						cerr<<"Error: Pattern("<<pat;
						cerr<<") contains a non-0/1 character(\'"<<pat[j]<<"\')."<<endl;
						_fecGrps.clear();
						return 0;
					}
					//1st readed pattern will be leftmost bit of signal 
					if(i==0) 
						_sigList.push_back( (size_t)(pat[j]-'0') );
					else
						_sigList[ofst+j]=(_sigList[ofst+j]<<1)+ (size_t)(pat[j]-'0');
				}
				num++;
			}
			else{
				finish=true;
				if(i==0) break;
				for(int j=0;j<_piList.size();j++)
					_sigList[ofst+j]=(_sigList[ofst+j]<<1) + (size_t)(0);	
			}
		}
		if(finish) break;
		ofst+=_piList.size();
	}
	return num;
}
void
CirMgr::simulate()
{
	int id0,id1;
	bool ph0,ph1;
	size_t w0,w1;
	_gateList[0]->setSignal(0);
	for(int i=0;i<_dfsList.size();i++){
		CirGate *g = _dfsList[i];
		if(g->getType()==AIG_GATE){
			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			id1 = g->getFaninGateID(1); ph1 = g->getFaninGatePhase(1);
			w0 = _gateList[id0]->getSignal(ph0);
			w1 = _gateList[id1]->getSignal(ph1);
			g->setSignal(w0&w1);
		}
		else if(g->getType()==PO_GATE){
			id0 = g->getFaninGateID(0); ph0 = g->getFaninGatePhase(0);
			w0 = _gateList[id0]->getSignal(ph0);
			g->setSignal(w0);
		}
		else if(g->getType()==UNDEF_GATE)
			g->setSignal(0);
	}
}
void
CirMgr::CreateFirstFEC()
{
	for(int i=0;i<_fecGrps.size();i++)
		delete _fecGrps[i];
	_fecGrps.clear();
	FECgroup *fgp = new FECgroup;
	//put all signal in one FECgroup
	//only AIG_GATE & CONST_GATE in FECgroup
	fgp->push_back(0);
	for(int i=0;i<_dfsList.size();i++){
		CirGate *g = _dfsList[i];
		if(g->getType()==AIG_GATE)
			fgp->push_back(g->getID()*2);
	}
	//add it into _fecGrps
	_fecGrps.push_back(fgp);
}
bool
CirMgr::IdentifyFEC()
{
	bool IdtfyNew=false;
	bool changed=false;
	for(int i=0;i<_fecGrps.size();i++){
		unordered_map<size_t,FECgroup*> newFecGrps;
		FECgroup *fecGrp=_fecGrps[i];
		changed=false;
		for(int j=0;j<fecGrp->size();j++){
			int id = fecGrp->at(j)/2; bool phase = fecGrp->at(j)%2;
			CirGate *g = _gateList[id];
			if(newFecGrps.find(g->getSignal(0))!=newFecGrps.end()){
				FECgroup *grp=newFecGrps[g->getSignal(0)];
				grp->push_back(id*2+(phase!=0));
				if(phase!=0) changed=true; //phase changed
			}
			else if(newFecGrps.find(g->getSignal(1))!=newFecGrps.end()){
				FECgroup *grp=newFecGrps[g->getSignal(1)];
				grp->push_back(id*2+(phase!=1));
				if(phase!=1) changed=true; //phase changed
			}
			else {
				FECgroup *newgrp = new FECgroup;
				newgrp->push_back(id*2+phase);
				newFecGrps[g->getSignal(phase)] = newgrp;
			}
		}
		if((changed && newFecGrps.size()==1)|| newFecGrps.size()>1 ){ 
			IdtfyNew=true;//identify new FECgroup
			delete _fecGrps[i];
			_fecGrps.erase(_fecGrps.begin()+i);i--;
		
			unordered_map<size_t,FECgroup*>::iterator it=newFecGrps.begin();
			for(;it!=newFecGrps.end();++it){
				if(it->second->size()>1)
					_fecGrps.push_back(it->second);
				//else singleton
			}
		}
	}
	return IdtfyNew;
}

void
CirMgr::writeSim(int num)
{
	string pat,result;
	for(int i=63;i>=0;i--){
		//1st pattern will be leftmost bit of signal
		pat.clear();result.clear();
		for(int j=0;j<_piList.size();j++){
			size_t bit = ((size_t)(1)<<i) & (_piList[j]->getSignal(0));
			bit = bit>>i;
			pat+= ('0'+bit);
		}
		for(int j=0;j<_poList.size();j++){
			size_t bit = ((size_t)(1)<<i) & (_poList[j]->getSignal(0));
			bit = bit>>i;
			result+= ('0'+bit);
		}
		if(num>0) (*_simLog)<<pat<<" "<<result<<endl;
		else break;
		num--;
	}
}

void
CirMgr::setFirstFgp() const {
	//Reset all gate's _fgp
	for(int i=0;i<_gateList.size();i++){
		if(_gateList[i]!=NULL)
			_gateList[i]->setFgp(NULL);
	}
	for(int i=0;i<_fecGrps.size();i++){
		FECgroup *grp = _fecGrps[i];
		int firstID = grp->at(0)/2;
		_gateList[firstID]->setFgp(grp);
	}
}
void
CirMgr::SortFEC(bool dfs){
	//sort each vector
	for(int i=0;i<_fecGrps.size();i++){
		if(_fecGrps[i]->size()<=1){
			delete _fecGrps[i];
			_fecGrps.erase(_fecGrps.begin()+i);i--;
			continue;
		}
		if(!dfs) 
			sort(_fecGrps[i]->begin(),_fecGrps[i]->end()); 
		else sort(_fecGrps[i]->begin(),_fecGrps[i]->end(),FecGrpSort());
	}
	if(dfs) sort(_fecGrps.begin(),_fecGrps.end(),FecListSort());
}
void
CirMgr::resetFEC(){
	for(int i=_fecGrps.size()-1;i>=0;i--){
		FECgroup *grp = _fecGrps[i];
		int id; 
		for(int j=grp->size()-1;j>=0;j--){
			id = grp->at(j)/2;
			if(_gateList[id]==NULL || _gateList[id]->getDfsNum()==-1){
				grp->erase(grp->begin()+j);
			}
			if(grp->size()==1){
				delete _fecGrps[i];
				_fecGrps.erase(_fecGrps.begin()+i);
				break;
			}
		}
	}
}
