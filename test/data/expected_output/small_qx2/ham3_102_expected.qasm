//Note: initial mapping (logical qubit at each location): 2, 1, 0, -1, -1, 
//Note: initial mapping (location of each logical qubit): 2, 1, 0, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
h q[2]; //cycle: 0 //h q[0]
t q[1]; //cycle: 0 //t q[1]
t q[0]; //cycle: 0 //t q[2]
cx q[0],q[1]; //cycle: 1 //cx q[2],q[1]
t q[2]; //cycle: 1 //t q[0]
cx q[2],q[0]; //cycle: 3 //cx q[0],q[2]
cx q[1],q[2]; //cycle: 5 //cx q[1],q[0]
tdg q[0]; //cycle: 5 //tdg q[2]
cx q[1],q[0]; //cycle: 7 //cx q[1],q[2]
t q[2]; //cycle: 7 //t q[0]
tdg q[1]; //cycle: 9 //tdg q[1]
tdg q[0]; //cycle: 9 //tdg q[2]
cx q[2],q[0]; //cycle: 10 //cx q[0],q[2]
cx q[1],q[2]; //cycle: 12 //cx q[1],q[0]
cx q[0],q[1]; //cycle: 14 //cx q[2],q[1]
h q[2]; //cycle: 14 //h q[0]
cx q[0],q[1]; //cycle: 16 //cx q[2],q[1]
cx q[1],q[0]; //cycle: 18 //cx q[1],q[2]
cx q[2],q[0]; //cycle: 20 //cx q[0],q[2]
cx q[0],q[1]; //cycle: 22 //cx q[2],q[1]
//20 original gates
//20 gates in generated circuit
//24 ideal depth (cycles)
//24 depth of generated circuit
//88 nodes popped from queue for processing.
//225 nodes remain in queue.
//HashFilter filtered 67 total nodes.
//HashFilter2 filtered 57 total nodes.
//HashFilter2 marked 56 total nodes.
