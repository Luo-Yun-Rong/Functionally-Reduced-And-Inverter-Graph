# Functionally-Reduced-And-Inverter-Graph
## Motivation
Functionally-Reduced-And-Inverter-Graph (FRAIG) is the final project of the course Data Structures and Programming in NTUEE, which is instructed by Professor Chung-Yang Huang. The FRAIG project is devised by Professor Chung-Yang Huang, and most of the implementation have been done. The files in the folder cir are my own implementation. These parts contain the core algorithms and fucntionality of this project. 

## Introduction to FRAIG
The FRAIG project reads an AIGER file. It performs trivial optimization such as unused gate sweeping and constant propagation. It uses hash to detect structurally equivalent signals in a circuit. It also performs Boolean logic simulation to identify potentially functionally equivalent pairs and calls the SAT solver to prove their functional equivalence. The command interface has been implemented by Professor Chung-Yang Huang, which is not included in this repository. This repository contains the implementation of the following command:
 
- CIRRead: read in a circuit (in the format of the AIGER file).
- CIRPrint: print the information of the citcuit, which includes the general information, the netlist of circuit if DFS order, the PI and PO of the circuit, the floating gates of the circuit, and the functionally equivalent 
- CIRGate
- CIRWrite
- CIRSWeep
- CIROPTimize
- CIRSTRash
- CIRSIMulate
- CIRFraig


## Implementation
The implementation is in C++.

## Reference


 
