//Note: initial mapping (logical qubit at each location): 0, 2, 3, 1, 4, 
//Note: initial mapping (location of each logical qubit): 0, 3, 1, 2, 4, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[3]; //cycle: 0 //cx q[3],q[1]
cx q[1],q[0]; //cycle: 0 //cx q[2],q[0]
cx q[3],q[4]; //cycle: 2 //cx q[1],q[4]
swp q[0],q[2]; //cycle: 2
t q[3]; //cycle: 4 //t q[1]
cx q[2],q[4]; //cycle: 8 //cx q[0],q[4]
h q[4]; //cycle: 10 //h q[4]
t q[2]; //cycle: 10 //t q[0]
cx q[2],q[3]; //cycle: 11 //cx q[0],q[1]
t q[4]; //cycle: 11 //t q[4]
cx q[4],q[2]; //cycle: 13 //cx q[4],q[0]
cx q[3],q[4]; //cycle: 15 //cx q[1],q[4]
tdg q[2]; //cycle: 15 //tdg q[0]
cx q[3],q[2]; //cycle: 17 //cx q[1],q[0]
t q[4]; //cycle: 17 //t q[4]
tdg q[3]; //cycle: 19 //tdg q[1]
tdg q[2]; //cycle: 19 //tdg q[0]
cx q[4],q[2]; //cycle: 20 //cx q[4],q[0]
cx q[3],q[4]; //cycle: 22 //cx q[1],q[4]
cx q[2],q[3]; //cycle: 24 //cx q[0],q[1]
h q[4]; //cycle: 24 //h q[4]
swp q[0],q[2]; //cycle: 26
swp q[3],q[4]; //cycle: 26
cx q[2],q[4]; //cycle: 32 //cx q[3],q[1]
cx q[1],q[0]; //cycle: 32 //cx q[2],q[0]
//22 original gates
//25 gates in generated circuit
//24 ideal depth (cycles)
//34 depth of generated circuit
//3204 nodes popped from queue for processing.
//1578 nodes remain in queue.
//HashFilter filtered 6614 total nodes.
//HashFilter2 filtered 906 total nodes.
//HashFilter2 marked 1115 total nodes.
