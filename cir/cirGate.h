/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Yun-Rong Luo, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

class CirGate;
class CirGateV;
class CirPiGate;
class CirPoGate;
class CirAigGate;
class CirConstGate;
class CirUndefGate;
class HashKey;
typedef vector<int> FECgroup;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGateV
{
	friend CirGate; friend CirAigGate; friend CirPoGate; friend HashKey;
	#define NEG 0x1
	CirGateV(){}
	CirGateV(CirGate *g, size_t phase):
		 _gateV(size_t(g) + phase) {}
	~CirGateV() {}
	CirGate* gate() const{
		 return (CirGate*)(_gateV & ~size_t(NEG));
	}
	bool isInv() const { return (_gateV & NEG); } //return 1 if inverted
	size_t _gateV;
};

class CirGate
{
public:
   CirGate(){}
   CirGate(size_t gateID, size_t lineNo, GateType type):
	   _gateID(gateID),_lineNo(lineNo),_type(type),_reachFromPo(false),_signal(0), 
	   _fgp(NULL), _dfsNum(-1),_faninList(NULL), _fanoutList(NULL),_sym(NULL),_ref(0){}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const;
   GateType getType() const { return _type; }
   unsigned getLineNo() const { return _lineNo; }
   size_t getID() const { return _gateID; }
   virtual bool isAig() const { return _type==AIG_GATE; }

   // Printing functions
   	//1. print Gate
   virtual void printGate() const = 0;
   void reportGate() const;
   	//2. print fanin
   void reportFanin(int level) const;
   void recurFanin(int level,int space) const;
   	//3. print fanout
   void reportFanout(int level) const;
   void recurFanout(int level,int space) const;

   //Fanin related
   void setFanin(size_t var); //store size_t(id  of gate)
   void setFanin(CirGate *g,size_t phase,int i);//store CirGateV
   void replaceFanin(size_t orgin, CirGate *g,size_t phase); 
   	//reset all of the fanin whose id=orgin,set to (g,phase)

   size_t FaninSize(){ 
	   if(_faninList==NULL) return 0;
	   else return _faninList -> size();
   }
    //1. called when circuit not connected 
   size_t getFanin(int i); //return _faninList -> at(i) (id of gate)
    //2. called after circuit connected
   size_t getFaninGateID(int i) { return _faninList -> at(i)->gate()->getID();}
   bool getFaninGatePhase(int i) { return _faninList -> at(i) -> isInv();}
   CirGateV* getFaninCirGateV(int i){ return _faninList -> at(i);}

   //Fanout related
   void setFanout(CirGate *g,size_t phase);
   void clearFanout(){ 
	   if(_fanoutList==NULL) return;
	   else _fanoutList->clear();
   }
   void removeFanout(size_t id); //remove all of the fanout=id 
   size_t FanoutSize(){
	   if(_fanoutList==NULL) return 0;
	   else return _fanoutList -> size();
   }
   size_t getFanoutGateID(const int &i){ return _fanoutList->at(i)->gate()->getID();}
   bool getFanoutGatePhase(const int &i){ return _fanoutList->at(i)->isInv();}


   //sym related
   void setSym(string &name) { _sym = new string(name);}
   string * getSym() {return _sym;}

   //dfs  related
   static void setglobalRef(){ _globalRef++; }
   void dfs(vector<CirGate*> &_dfsList);

   //Reachable from Po
   void setReach(bool reach){ _reachFromPo = reach;}
   bool getReach() const { return _reachFromPo;}

   //Signal
   void setSignal(const size_t &sig){ _signal = sig; }
   size_t getSignal(const bool &phase) const { 
	   if(phase) return ~_signal; 
	   else return _signal;
   }
   //FECgroup related
   void setFgp(FECgroup *fgp){ _fgp = fgp; }
   FECgroup* getFgp() const { return _fgp; }

   //Var related
   void setVar(const Var &v){ _var = v; }
   Var getVar() const { return _var; }

   //_dfsNum related
   void setDfsNum(const int& n) { _dfsNum=n; }
   int getDfsNum() const { return _dfsNum; }

private:
   //Gate physical information
   size_t _gateID;
   size_t _lineNo;
   GateType _type;
   bool _reachFromPo;
   size_t _signal;
   FECgroup *_fgp;
   Var _var; //sat var
   int _dfsNum;

   //Gate dfs information
   size_t _ref;
   static size_t _globalRef;

protected:
   string  *_sym;
   vector<CirGateV*> *_faninList;
   vector<CirGateV*> *_fanoutList;
};

class CirPiGate: public CirGate
{
public:
	CirPiGate(){}
	CirPiGate(size_t gateID,size_t lineNo):CirGate(gateID,lineNo,PI_GATE) {}
	~CirPiGate() {
		assert(_faninList == NULL);
		if(_fanoutList!=NULL) delete _fanoutList;
		if(_sym!=NULL) delete _sym;
	}

	void printGate()const{
		cout<<" PI  "<<getID();
		if(_sym!=NULL) cout<<" ("<<*_sym<<")";
		cout<<endl;
	}
};

class CirPoGate: public CirGate
{
public:
	CirPoGate(){}
	CirPoGate(size_t gateID,size_t lineNo):CirGate(gateID,lineNo,PO_GATE) {}
	~CirPoGate() {
		assert(_fanoutList == NULL);
		if(_faninList!=NULL) delete _faninList;
		if(_sym!=NULL) delete _sym;
	}

	void printGate()const{
		cout<<" PO  "<<getID()<<" ";

		CirGate *g =  _faninList -> at(0) -> gate();
		if(g->getType()==UNDEF_GATE) cout<<"*";
		if(_faninList->at(0)->isInv()) cout<<"!";
		cout<<g->getID();

		if(_sym!=NULL) cout<<" ("<<*_sym<<")";
		cout<<endl;
	}
};
class CirAigGate: public CirGate
{
public:
	CirAigGate(){}
	CirAigGate(size_t gateID,size_t lineNo):CirGate(gateID,lineNo,AIG_GATE){}
	~CirAigGate() {
		assert(_sym==NULL);
		if(_faninList!=NULL) delete _faninList;
		if(_fanoutList!=NULL) delete _fanoutList;
	}

	void printGate()const{
		assert(_sym==NULL);
		cout<<" AIG "<<getID();
		for(int i=0;i<2;i++){
			cout<<" ";
			CirGate *g =  _faninList -> at(i) -> gate();
			if(g->getType()==UNDEF_GATE) cout<<"*";
			if(_faninList->at(i)->isInv()) cout<<"!";
			cout<<g->getID();
		}
		cout<<endl;
	}
};
class CirConstGate: public CirGate
{
public:
	CirConstGate(){}
	CirConstGate(size_t gateID,size_t lineNo):CirGate(gateID,lineNo,CONST_GATE){}

	~CirConstGate(){
		assert(_faninList == NULL);
		if(_fanoutList!=NULL) delete _fanoutList;
		if(_sym!=NULL) delete _sym;
	}
	void printGate() const{ cout<<" CONST0"<<endl;}
};
class CirUndefGate: public CirGate
{
public:
	CirUndefGate(){}
	CirUndefGate(size_t gateID,size_t lineNo): CirGate(gateID,lineNo,UNDEF_GATE){}

	~CirUndefGate(){
		assert(_faninList==NULL);
		if(_fanoutList!=NULL) delete _fanoutList;
		assert(_sym ==NULL);
	}
	void printGate() const{}
};


#endif // CIR_GATE_H
