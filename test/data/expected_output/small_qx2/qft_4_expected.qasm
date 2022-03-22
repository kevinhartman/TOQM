//Note: initial mapping (logical qubit at each location): 1, 2, 0, -1, 3, 
//Note: initial mapping (location of each logical qubit): 2, 0, 1, 4, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[0]; //cycle: 0 //cx q[0],q[1]
cx q[2],q[1]; //cycle: 2 //cx q[0],q[2]
cx q[0],q[1]; //cycle: 4 //cx q[1],q[2]
swap q[2],q[4]; //cycle: 4
cx q[4],q[2]; //cycle: 10 //cx q[0],q[3]
cx q[0],q[2]; //cycle: 12 //cx q[1],q[3]
cx q[1],q[2]; //cycle: 14 //cx q[2],q[3]
//6 original gates
//7 gates in generated circuit
//10 ideal depth (cycles)
//16 depth of generated circuit
//453 nodes popped from queue for processing.
//549 nodes remain in queue.
//HashFilter filtered 2444 total nodes.
//HashFilter2 filtered 67 total nodes.
//HashFilter2 marked 218 total nodes.
