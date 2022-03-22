//Note: initial mapping (logical qubit at each location): 2, 1, 0, -1, -1, 
//Note: initial mapping (location of each logical qubit): 2, 1, 0, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[0],q[1]; //cycle: 0 //cx q[2],q[1]
t q[2]; //cycle: 0 //t q[0]
h q[0]; //cycle: 2 //h q[2]
t q[1]; //cycle: 2 //t q[1]
cx q[2],q[1]; //cycle: 3 //cx q[0],q[1]
t q[0]; //cycle: 3 //t q[2]
cx q[0],q[2]; //cycle: 5 //cx q[2],q[0]
cx q[1],q[0]; //cycle: 7 //cx q[1],q[2]
tdg q[2]; //cycle: 7 //tdg q[0]
cx q[1],q[2]; //cycle: 9 //cx q[1],q[0]
t q[0]; //cycle: 9 //t q[2]
tdg q[1]; //cycle: 11 //tdg q[1]
tdg q[2]; //cycle: 11 //tdg q[0]
cx q[0],q[2]; //cycle: 12 //cx q[2],q[0]
cx q[1],q[0]; //cycle: 14 //cx q[1],q[2]
cx q[2],q[1]; //cycle: 16 //cx q[0],q[1]
h q[0]; //cycle: 16 //h q[2]
t q[0]; //cycle: 17 //t q[2]
h q[2]; //cycle: 18 //h q[0]
t q[1]; //cycle: 18 //t q[1]
cx q[1],q[0]; //cycle: 19 //cx q[1],q[2]
t q[2]; //cycle: 19 //t q[0]
cx q[2],q[1]; //cycle: 21 //cx q[0],q[1]
cx q[0],q[2]; //cycle: 23 //cx q[2],q[0]
tdg q[1]; //cycle: 23 //tdg q[1]
cx q[0],q[1]; //cycle: 25 //cx q[2],q[1]
t q[2]; //cycle: 25 //t q[0]
tdg q[0]; //cycle: 27 //tdg q[2]
tdg q[1]; //cycle: 27 //tdg q[1]
cx q[2],q[1]; //cycle: 28 //cx q[0],q[1]
cx q[0],q[2]; //cycle: 30 //cx q[2],q[0]
cx q[1],q[0]; //cycle: 32 //cx q[1],q[2]
h q[2]; //cycle: 32 //h q[0]
t q[2]; //cycle: 33 //t q[0]
h q[0]; //cycle: 34 //h q[2]
t q[1]; //cycle: 34 //t q[1]
cx q[2],q[1]; //cycle: 35 //cx q[0],q[1]
t q[0]; //cycle: 35 //t q[2]
cx q[0],q[2]; //cycle: 37 //cx q[2],q[0]
cx q[1],q[0]; //cycle: 39 //cx q[1],q[2]
tdg q[2]; //cycle: 39 //tdg q[0]
cx q[1],q[2]; //cycle: 41 //cx q[1],q[0]
t q[0]; //cycle: 41 //t q[2]
tdg q[1]; //cycle: 43 //tdg q[1]
tdg q[2]; //cycle: 43 //tdg q[0]
cx q[0],q[2]; //cycle: 44 //cx q[2],q[0]
cx q[1],q[0]; //cycle: 46 //cx q[1],q[2]
cx q[2],q[1]; //cycle: 48 //cx q[0],q[1]
h q[0]; //cycle: 48 //h q[2]
cx q[0],q[1]; //cycle: 50 //cx q[2],q[1]
//50 original gates
//50 gates in generated circuit
//52 ideal depth (cycles)
//52 depth of generated circuit
//78 nodes popped from queue for processing.
//322 nodes remain in queue.
//HashFilter filtered 86 total nodes.
//HashFilter2 filtered 51 total nodes.
//HashFilter2 marked 37 total nodes.
