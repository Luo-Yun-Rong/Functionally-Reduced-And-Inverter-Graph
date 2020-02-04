/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Yun-Rong Luo ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;
//constant zero gate
CirGate *CirMgr::_const0 = new CirConstGate(0,0);

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for Access        		  */
/**************************************************************/
CirGate* 
CirMgr::getGate(unsigned gid) const{
	if(gid>= _gateList.size()|| gid<0 || _gateList[gid]==NULL)
		return NULL;
	return _gateList[gid];
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
	ifstream fin(fileName);
	if(readHeader(fin) && readInput(fin) && readOutput(fin)  
		&& readAIG(fin) && readSym(fin) && readComment(fin)){
		connect();
		dfs();
		return true;
	}
   return false;
}
bool
CirMgr::readHeader(ifstream &fin){
	string aag;
	fin>>aag>>M>>I>>L>>O>>A;
	return true;
}
bool
CirMgr::readInput(ifstream &fin){
	size_t gateID,lineNo;
	for(int i=0;i<I;i++){
		fin>>gateID; gateID = gateID>>1; //gateID = gateID/2
		lineNo = i+2; //when i=0,lineNo=2
		CirPiGate  *pi =  new CirPiGate(gateID,lineNo);
		_piList.push_back(pi);
		if(_gateList.size()<gateID+1) _gateList.resize(gateID+1,NULL);
		_gateList[gateID] = pi;
	}
	return true;
}
bool
CirMgr::readOutput(ifstream &fin){
	size_t gateID,lineNo;
	size_t var; //var will be set as fanin of gate gateID
	for(int i=0;i<O;i++){
		gateID = M+1+i;
		lineNo = i+I+2; //when I=1,i=0,lineNo = 3
		CirPoGate *po = new CirPoGate(gateID,lineNo);
		fin>>var; po -> setFanin(var);
		_poList.push_back(po);
		if(_gateList.size()<gateID+1) _gateList.resize(gateID+1,NULL);
		_gateList[gateID] = po;
	}
	return true;
}
bool
CirMgr::readAIG(ifstream &fin){
	size_t gateID,lineNo;
	size_t var1,var2; //var1,var2 will be set as fanin of gate gateID
	for(int i=0;i<A;i++){
		fin>>gateID>>var1>>var2;
		gateID=gateID>>1;
		lineNo = i+I+O+2;
		CirGate *a = new CirAigGate(gateID,lineNo);
		a -> setFanin(var1); a -> setFanin(var2);
		if(_gateList.size()<gateID+1) _gateList.resize(gateID+1,NULL);
		_gateList[gateID] = a;
	}
	return true;
}
bool
CirMgr::readSym(ifstream &fin){
	string pos, name;
	int id;
	getline(fin,pos); //to remove the '\n' of previos line
	while(getline(fin,pos,' ') && getline(fin,name)){
		if(pos[0]!='i' && pos[0]!='o') break;
		if(pos[0]=='i'){
			id = stoi(pos.erase(0,1)); //i0 is
			assert(id<_piList.size());
			_piList[id] -> setSym(name);
		}
		else if(pos[0]=='o'){
			id = stoi(pos.erase(0,1)); //i0 is
			assert(id<_poList.size());
			_poList[id] -> setSym(name);
		}
	}
	return true;
}
bool
CirMgr::readComment(ifstream &fin){
	 string comment;
	 while(fin>>comment){}
	 return true;
}
void
CirMgr::connect(){
	//set const zero
	_gateList[0] = _const0;
	//connect fanin/fanout,check floating
	for(int i=0;i<_gateList.size();i++){
		CirGate  *g = _gateList[i];
		if(g==NULL) continue;
		else if(g->getType()==PO_GATE || g->getType()==AIG_GATE){
			for(int j=0;j<g->FaninSize();j++){
				size_t n = g->getFanin(j);
				size_t id = n/2; size_t phase = n%2;
				//floating case: b,c,d
				if(id>= _gateList.size()) 
					_gateList.resize(id+1,NULL);
				if(_gateList[id] == NULL 
					|| _gateList[id]-> getType() == UNDEF_GATE) {
					if(_floatList.empty()||_floatList.back()!=g)
						_floatList.push_back(g);//prevent repeat
					if(_gateList[id]== NULL) 
						_gateList[id] = new CirUndefGate(id,0);
				}
				//even undefined gate has to set fanout & fanin
				_gateList[id] -> setFanout(g,phase);		
				g -> setFanin(_gateList[id],phase,j);
			}
		}
	}
	//check undefined: case a,c
	for(int i=0;i<_gateList.size();i++){
		CirGate *g = _gateList[i];
		if(g == NULL) continue;
		else if(g -> getType() == PI_GATE 
				|| g-> getType()== AIG_GATE){
			if(g -> FanoutSize() == 0)
				_unuseList.push_back(g);
		}
	}
}
void
CirMgr::dfs(){
	CirGate::setglobalRef();
	for(int i=0;i<_poList.size();i++)
		_poList[i] -> dfs(_dfsList);
	Aw=0;
	for(int i=0;i<_dfsList.size();i++){
		_dfsList[i]->setDfsNum(i);
		if(_dfsList[i]->getType()==AIG_GATE)
			Aw++;
	}
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
	cout<<endl;
	cout<<"Circuit Statistics"<<endl;
	cout<<"=================="<<endl;
	cout<<"  PI"<<setw(12)<<I<<endl;
	cout<<"  PO"<<setw(12)<<O<<endl;
	cout<<"  AIG"<<setw(11)<<A<<endl;
	cout<<"------------------"<<endl;
	cout<<"  Total"<<setw(9)<<O+I+A<<endl;
}

void
CirMgr::printNetlist() const
{

   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "]";
      _dfsList[i]->printGate();
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i=0;i<I;i++)
	   cout<<" "<<_piList[i] -> getID();
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i=0;i<O;i++)
	   cout<<" "<<_poList[i] -> getID();
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
	if(_floatList.size()){
		cout<<"Gates with floating fanin(s):";
		for(int i=0;i<_floatList.size();i++)
			cout<<" "<<_floatList[i]->getID();
		cout<<endl;
	}
	if(_unuseList.size()){
		cout<<"Gates defined but not used  :";
		for(int i=0;i<_unuseList.size();i++)
			cout<<" "<<_unuseList[i]->getID();
		cout<<endl;
	}
}

void
CirMgr::printFECPairs() const
{
	setFirstFgp();
	//Print
	int n=0;
	for(int i=0;i<_gateList.size();i++){
		CirGate *g = _gateList[i];
		if(g!=NULL && g->getFgp()!=NULL){
			FECgroup *grp = g->getFgp();
			cout<<"["<<n<<"]";
			bool fstPhase = grp->at(0)%2;
			for(int j=0;j<grp->size();j++){
				int id = grp->at(j)/2; bool phase = grp->at(j)%2;
				if(phase!=fstPhase) cout<<" !"<<id; //no ! before first id
				else cout<<" "<<id;
			}
			cout<<endl;
			n++;
		}
	}
}

void
CirMgr::writeAag(ostream& outfile) const
{
	outfile<<"aag "<<M<<" "<<I<<" "<<L<<" "<<O<<" "<<Aw<<endl;
	//PI
	for(int i=0;i<I;i++)
		outfile<<(_piList[i]->getID())*2<<endl;
	//PO
	for(int i=0;i<O;i++){
		int id= _poList[i] ->getFaninGateID(0);
		bool phase = _poList[i]-> getFaninGatePhase(0);
		outfile<<id*2+phase<<endl;
	}
	//AIG
	for(int i=0;i<_dfsList.size();i++){
		if(_dfsList[i]!=NULL && _dfsList[i] -> getType()==AIG_GATE){
			outfile<<(_dfsList[i]-> getID())*2;
			for(int j=0;j<2;j++){
				outfile<<" ";
				int id= _dfsList[i] ->getFaninGateID(j) ;
				bool phase = _dfsList[i]-> getFaninGatePhase(j);
				outfile<<id*2+phase;
			}
			outfile<<endl;
		}
	}
	//symbol
	for(int i=0;i<I;i++){
		if(_piList[i]->getSym()!=NULL )
			outfile<<"i"<<i<<" "<<*(_piList[i]->getSym())<<endl;
	}
	for(int i=0;i<O;i++){
		if(_poList[i]->getSym()!=NULL)
			outfile<<"o"<<i<<" "<<*(_poList[i]->getSym())<<endl;
	}
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
	vector<int> piCone;
	vector<CirGate*> dfsCone;
	CirGate::setglobalRef();
	g->dfs(dfsCone);
	int Mc=0,Ic=0,Lc=0,Oc=1,Ac=0;
	for(int i=0;i<dfsCone.size();i++){
		if(dfsCone[i]->getType()==PI_GATE){
			Ic++;
			piCone.push_back(dfsCone[i]->getID());
		}
		else if(dfsCone[i]->getType()==AIG_GATE)
			Ac++;
		if(dfsCone[i]->getID()>Mc) Mc = dfsCone[i]->getID();
	}
	outfile<<"aag "<<Mc<<" "<<Ic<<" "<<Lc<<" "<<Oc<<" "<<Ac<<endl;
	//PI
	sort(piCone.begin(),piCone.end());
	for(int i=0;i<Ic;i++)
		outfile<<(piCone[i]*2)<<endl;
	//PO
	outfile<<(g->getID()*2)<<endl;
	//AIG
	for(int i=0;i<dfsCone.size();i++){
		if(dfsCone[i] -> getType()==AIG_GATE){
			outfile<<(dfsCone[i]-> getID())*2;
			for(int j=0;j<2;j++){
				outfile<<" ";
				int id= dfsCone[i] ->getFaninGateID(j) ;
				bool phase = dfsCone[i]-> getFaninGatePhase(j);
				outfile<<id*2+phase;
			}
			outfile<<endl;
		}
	}
	//symbol
	for(int i=0;i<Ic;i++){
		if(_gateList[ piCone[i] ]->getSym()!=NULL )
			outfile<<"i"<<i<<" "<<*(_gateList[piCone[i]]->getSym())<<endl;
	}
	outfile<<"o0 "<<g->getID()<<endl;
}

