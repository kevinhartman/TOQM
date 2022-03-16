//Note: initial mapping (logical qubit at each location): 0, 1, 2, -1, -1, 
//Note: initial mapping (location of each logical qubit): 0, 1, 2, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[0],q[1]; //cycle: 0 //cx q[0],q[1]
t q[2]; //cycle: 0 //t q[2]
h q[0]; //cycle: 2 //h q[0]
t q[1]; //cycle: 2 //t q[1]
cx q[2],q[1]; //cycle: 3 //cx q[2],q[1]
t q[0]; //cycle: 3 //t q[0]
cx q[0],q[2]; //cycle: 5 //cx q[0],q[2]
cx q[1],q[0]; //cycle: 7 //cx q[1],q[0]
tdg q[2]; //cycle: 7 //tdg q[2]
cx q[1],q[2]; //cycle: 9 //cx q[1],q[2]
t q[0]; //cycle: 9 //t q[0]
tdg q[1]; //cycle: 11 //tdg q[1]
tdg q[2]; //cycle: 11 //tdg q[2]
cx q[0],q[2]; //cycle: 12 //cx q[0],q[2]
cx q[1],q[0]; //cycle: 14 //cx q[1],q[0]
cx q[2],q[1]; //cycle: 16 //cx q[2],q[1]
h q[0]; //cycle: 16 //h q[0]
cx q[0],q[1]; //cycle: 18 //cx q[0],q[1]
x q[0]; //cycle: 20 //x q[0]
//19 original gates
//19 gates in generated circuit
//21 ideal depth (cycles)
//21 depth of generated circuit
//26 nodes popped from queue for processing.
//117 nodes remain in queue.
//HashFilter filtered 80 total nodes.
//HashFilter2 filtered 0 total nodes.
//HashFilter2 marked 0 total nodes.
