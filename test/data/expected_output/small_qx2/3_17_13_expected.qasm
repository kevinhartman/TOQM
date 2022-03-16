//Note: initial mapping (logical qubit at each location): 0, 1, 2, -1, -1, 
//Note: initial mapping (location of each logical qubit): 0, 1, 2, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
x q[2]; //cycle: 0 //x q[2]
cx q[0],q[2]; //cycle: 1 //cx q[0],q[2]
cx q[2],q[1]; //cycle: 3 //cx q[2],q[1]
h q[0]; //cycle: 3 //h q[0]
t q[0]; //cycle: 4 //t q[0]
t q[1]; //cycle: 5 //t q[1]
t q[2]; //cycle: 5 //t q[2]
cx q[2],q[1]; //cycle: 6 //cx q[2],q[1]
cx q[0],q[2]; //cycle: 8 //cx q[0],q[2]
cx q[1],q[0]; //cycle: 10 //cx q[1],q[0]
tdg q[2]; //cycle: 10 //tdg q[2]
cx q[1],q[2]; //cycle: 12 //cx q[1],q[2]
t q[0]; //cycle: 12 //t q[0]
tdg q[1]; //cycle: 14 //tdg q[1]
tdg q[2]; //cycle: 14 //tdg q[2]
cx q[0],q[2]; //cycle: 15 //cx q[0],q[2]
cx q[1],q[0]; //cycle: 17 //cx q[1],q[0]
cx q[2],q[1]; //cycle: 19 //cx q[2],q[1]
h q[0]; //cycle: 19 //h q[0]
t q[0]; //cycle: 20 //t q[0]
h q[2]; //cycle: 21 //h q[2]
t q[1]; //cycle: 21 //t q[1]
cx q[1],q[0]; //cycle: 22 //cx q[1],q[0]
t q[2]; //cycle: 22 //t q[2]
cx q[2],q[1]; //cycle: 24 //cx q[2],q[1]
cx q[0],q[2]; //cycle: 26 //cx q[0],q[2]
tdg q[1]; //cycle: 26 //tdg q[1]
cx q[0],q[1]; //cycle: 28 //cx q[0],q[1]
t q[2]; //cycle: 28 //t q[2]
tdg q[0]; //cycle: 30 //tdg q[0]
tdg q[1]; //cycle: 30 //tdg q[1]
cx q[2],q[1]; //cycle: 31 //cx q[2],q[1]
cx q[0],q[2]; //cycle: 33 //cx q[0],q[2]
cx q[1],q[0]; //cycle: 35 //cx q[1],q[0]
h q[2]; //cycle: 35 //h q[2]
cx q[1],q[2]; //cycle: 37 //cx q[1],q[2]
//36 original gates
//36 gates in generated circuit
//39 ideal depth (cycles)
//39 depth of generated circuit
//57 nodes popped from queue for processing.
//248 nodes remain in queue.
//HashFilter filtered 135 total nodes.
//HashFilter2 filtered 18 total nodes.
//HashFilter2 marked 15 total nodes.
