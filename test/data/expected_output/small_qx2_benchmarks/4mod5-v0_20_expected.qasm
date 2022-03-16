//Note: initial mapping (logical qubit at each location): 2, 4, 0, 1, 3, 
//Note: initial mapping (location of each logical qubit): 2, 3, 0, 4, 1, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[4],q[3]; //cycle: 0 //cx q[3],q[1]
x q[2]; //cycle: 0 //x q[0]
h q[1]; //cycle: 0 //h q[4]
cx q[2],q[0]; //cycle: 1 //cx q[0],q[2]
t q[1]; //cycle: 1 //t q[4]
swap q[2],q[3]; //cycle: 3
cx q[0],q[2]; //cycle: 9 //cx q[2],q[1]
t q[0]; //cycle: 11 //t q[2]
t q[2]; //cycle: 11 //t q[1]
cx q[2],q[0]; //cycle: 12 //cx q[1],q[2]
cx q[1],q[2]; //cycle: 14 //cx q[4],q[1]
cx q[0],q[1]; //cycle: 16 //cx q[2],q[4]
tdg q[2]; //cycle: 16 //tdg q[1]
cx q[0],q[2]; //cycle: 18 //cx q[2],q[1]
t q[1]; //cycle: 18 //t q[4]
tdg q[0]; //cycle: 20 //tdg q[2]
tdg q[2]; //cycle: 20 //tdg q[1]
cx q[1],q[2]; //cycle: 21 //cx q[4],q[1]
cx q[0],q[1]; //cycle: 23 //cx q[2],q[4]
cx q[2],q[0]; //cycle: 25 //cx q[1],q[2]
h q[1]; //cycle: 25 //h q[4]
//20 original gates
//21 gates in generated circuit
//21 ideal depth (cycles)
//27 depth of generated circuit
//1781 nodes popped from queue for processing.
//2108 nodes remain in queue.
//HashFilter filtered 4108 total nodes.
//HashFilter2 filtered 1120 total nodes.
//HashFilter2 marked 1984 total nodes.
