//Note: initial mapping (logical qubit at each location): 3, 2, 1, 4, 0, 
//Note: initial mapping (location of each logical qubit): 4, 2, 1, 0, 3, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[1],q[2]; //cycle: 0 //cx q[2],q[1]
h q[4]; //cycle: 0 //h q[0]
t q[4]; //cycle: 1 //t q[0]
cx q[2],q[1]; //cycle: 2 //cx q[1],q[2]
cx q[0],q[1]; //cycle: 4 //cx q[3],q[2]
swp q[2],q[3]; //cycle: 4
cx q[1],q[0]; //cycle: 6 //cx q[2],q[3]
cx q[2],q[0]; //cycle: 10 //cx q[4],q[3]
t q[3]; //cycle: 10 //t q[1]
cx q[0],q[2]; //cycle: 12 //cx q[3],q[4]
t q[2]; //cycle: 14 //t q[4]
cx q[2],q[3]; //cycle: 15 //cx q[4],q[1]
cx q[4],q[2]; //cycle: 17 //cx q[0],q[4]
cx q[3],q[4]; //cycle: 19 //cx q[1],q[0]
tdg q[2]; //cycle: 19 //tdg q[4]
cx q[3],q[2]; //cycle: 21 //cx q[1],q[4]
t q[4]; //cycle: 21 //t q[0]
tdg q[3]; //cycle: 23 //tdg q[1]
tdg q[2]; //cycle: 23 //tdg q[4]
cx q[4],q[2]; //cycle: 24 //cx q[0],q[4]
cx q[3],q[4]; //cycle: 26 //cx q[1],q[0]
cx q[2],q[3]; //cycle: 28 //cx q[4],q[1]
h q[4]; //cycle: 28 //h q[0]
cx q[2],q[0]; //cycle: 30 //cx q[4],q[3]
cx q[2],q[1]; //cycle: 32 //cx q[4],q[2]
cx q[2],q[3]; //cycle: 34 //cx q[4],q[1]
cx q[4],q[2]; //cycle: 36 //cx q[0],q[4]
cx q[2],q[4]; //cycle: 38 //cx q[4],q[0]
//27 original gates
//28 gates in generated circuit
//38 ideal depth (cycles)
//40 depth of generated circuit
//836 nodes popped from queue for processing.
//1225 nodes remain in queue.
//HashFilter filtered 2280 total nodes.
//HashFilter2 filtered 432 total nodes.
//HashFilter2 marked 935 total nodes.
