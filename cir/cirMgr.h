/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Yun-Rong Luo, Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "cirGate.h"
#include "sat.h"
#include "cirDef.h"

using namespace std;


extern CirMgr *cirMgr;

typedef vector<int> FECgroup;

class CirMgr
{
public:
   friend class FecListSort;
   friend class FecGrpSort;
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();
   
   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

private:
   //private Member function about reading
   bool readHeader(ifstream &fin);
   bool readInput(ifstream &fin);
   bool readOutput(ifstream &fin);
   bool readAIG(ifstream &fin);
   bool readSym(ifstream &fin);
   bool readComment(ifstream &fin);
   void connect();
   void dfs();
   
   //private Member functions about optimization
   void resetFloat(bool cirsw = false);
   void resetUnuse();
   void resetDfs();
   void mergeGate(CirGate* delGate, CirGate *merGate,int propPhase=-1);

   //private Member functions about simulation
   void randSig();
   int readSig(ifstream &fin);
   void simulate();
   void CreateFirstFEC();
   bool IdentifyFEC();
   void writeSim(int num);
   void setFirstFgp() const;
   void SortFEC(bool dfs);
   void resetFEC();
   
   //private Member functions about fraig
   void genProofModel(SatSolver& s);
   bool ProvePair(SatSolver &solver,int id0,bool ph0,int id1,bool ph1);
   void collectPattern(SatSolver &solver,int numSig);

   //private Member variable
   ofstream           *_simLog; 
   int M,I,L,O,A,Aw; //Aw is for write operation
   static CirGate 		*_const0;
   vector<CirPiGate*> 	_piList;
   vector<CirPoGate*> 	_poList;
   vector<CirGate*> 	_gateList;
   vector<CirGate*> 	_floatList;
   vector<CirGate*>		_unuseList;
   vector<CirGate*>		_dfsList;
   vector <size_t> 		_sigList;
   vector<FECgroup*>	_fecGrps;
};

#endif // CIR_MGR_H
