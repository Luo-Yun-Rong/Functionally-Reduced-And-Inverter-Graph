# Functionally-Reduced-And-Inverter-Graph
## Motivation
Functionally Reduced And Inverter Graph (FRAIG) is the final project of the course Data Structures and Programming in NTUEE, which is instructed by Professor Chung-Yang (Ric) Huang. The FRAIG project is devised by Professor Chung-Yang (Ric) Huang, and most of the implementation has been done. The files in the folder /cir are my implementation. These parts contain the core algorithms and functionality of this project.

## Introduction to FRAIG
The FRAIG project reads an AIGER file. It performs trivial optimization such as unused gate sweeping and constant propagation. It uses hash to detect structurally equivalent signals in a circuit. It also performs Boolean logic simulation to identify the functionally equivalent candidate (FEC) pairs and calls the SAT solver to prove their functional equivalence. The command interface has been implemented by Professor Chung-Yang (Ric) Huang, which is not included in this repository. The SAT solver we used in this project is MiniSat. This repository contains the implementation of the following commands:
 
- CIRRead:     Read in a circuit , which is in the format of the AIGER file.
- CIRPrint:    Print the information of the circuit, which includes the general information, the netlist of circuit in the DFS 
               order, the PI and PO of the circuit, the floating gates of the circuit, and the functionally equivalent 
               candidate pairs.
- CIRGate:     Report the information of a gate, including the gate ID, its fanin and fanout, and the FEC partner of the gate.
- CIRWrite:    Write the netlist to an AIGER file (.aag)
- CIRSWeep:    Remove the unued gates in the circuit.
- CIROPTimize: Perform trivial optimization, i.e. constant propagation.
- CIRSTRash:   Perform structural hash on the circuit netlist and merge the structurally equivalent gates.
- CIRSIMulate: Perform Boolean logic simulation on the circuit and detect the FEC pairs.
- CIRFraig:    Call the SAT solver to prove the functional equivalence of the FEC pairs in the circuit.

## Reference
- Professor Chung-Yang (Ric) Huang's lecture notes and the project specification. 
- MiniSat web page: http://minisat.se



 
