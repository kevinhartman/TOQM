//Note: initial mapping (logical qubit at each location): 2, 1, 0, 3, 4, 
//Note: initial mapping (location of each logical qubit): 2, 1, 0, 3, 4, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
swap q[3],q[4]; //cycle: 0
h q[2]; //cycle: 0 //h q[0]
t q[0]; //cycle: 0 //t q[2]
t q[1]; //cycle: 0 //t q[1]
cx q[1],q[0]; //cycle: 1 //cx q[1],q[2]
t q[2]; //cycle: 1 //t q[0]
cx q[2],q[1]; //cycle: 3 //cx q[0],q[1]
cx q[0],q[2]; //cycle: 5 //cx q[2],q[0]
tdg q[1]; //cycle: 5 //tdg q[1]
cx q[0],q[1]; //cycle: 7 //cx q[2],q[1]
t q[2]; //cycle: 7 //t q[0]
tdg q[0]; //cycle: 9 //tdg q[2]
tdg q[1]; //cycle: 9 //tdg q[1]
cx q[2],q[1]; //cycle: 10 //cx q[0],q[1]
cx q[0],q[2]; //cycle: 12 //cx q[2],q[0]
cx q[1],q[0]; //cycle: 14 //cx q[1],q[2]
h q[2]; //cycle: 14 //h q[0]
cx q[3],q[2]; //cycle: 15 //cx q[4],q[0]
cx q[2],q[3]; //cycle: 17 //cx q[0],q[4]
//18 original gates
//19 gates in generated circuit
//19 ideal depth (cycles)
//19 depth of generated circuit
//91 nodes popped from queue for processing.
//284 nodes remain in queue.
//HashFilter filtered 132 total nodes.
//HashFilter2 filtered 56 total nodes.
//HashFilter2 marked 74 total nodes.
