# Functionally-Reduced-And-Inverter-Graph
## Motivation
Functionally-Reduced-And-Inverter-Graph (FRAIG) is the final project of the course Data Structures and Programming in NTUEE, which is instructed by Professor Chung-Yang Huang. The FRAIG project is devised by Professor Chung-Yang Huang, and most of the implementation have been done. The files in the folder cir are my own implementation. These parts contain the core algorithms and fucntionality of this project. 

## Introduction to FRAIG
The FRAIG project reads an AIGER file. It performs trivial optimization such as unused gate sweeping and constant propagation. It uses hash to detect structurally equivalent signals in a circuit. It also performs Boolean logic simulation to identify the functionally equivalent candidate (FEC) pairs and calls the SAT solver to prove their functional equivalence. The command interface has been implemented by Professor Chung-Yang Huang, which is not included in this repository. The SAT solver we used in this project is MiniSAT. This repository contains the implementation of the following command:
 
- CIRRead:     Read in a circuit (in the format of the AIGER file).
- CIRPrint:    Print the information of the citcuit, which includes the general information, the netlist of circuit if DFS 
               order, the PI and PO of the circuit, the floating gates of the circuit, and the functionally equivalent  
               candidate pairs.
- CIRGate:     Report the information of a gate, including the gate ID, its fanin and fanout, and the FEC partner of the gate.
- CIRWrite:    Write the netlist to an ASCII AIG file (.aag)
- CIRSWeep:    Remove the unsued gates in the circuit.
- CIROPTimize: Perform trivial optimixation. That is, constant propagation.
- CIRSTRash:   Perform structural hash on the circuit netlist.
- CIRSIMulate: Perform Boolean logic simulation on the circuit.
- CIRFraig:    Calls the SAT solver to prove the functionally equivalence of the FEC pairs in the circuit.


## Implementation
The implementation is in C++.

## Reference
The reference of this work is Professor Chung-Yang Huang's lecture notes and the project specification. 


 
