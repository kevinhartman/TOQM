//Note: initial mapping (logical qubit at each location): 1, 0, 2, -1, -1, 
//Note: initial mapping (location of each logical qubit): 1, 0, 2, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[0]; //cycle: 0 //cx q[2],q[1]
t q[1]; //cycle: 0 //t q[0]
h q[2]; //cycle: 2 //h q[2]
t q[0]; //cycle: 2 //t q[1]
cx q[1],q[0]; //cycle: 3 //cx q[0],q[1]
t q[2]; //cycle: 3 //t q[2]
cx q[2],q[1]; //cycle: 5 //cx q[2],q[0]
cx q[0],q[2]; //cycle: 7 //cx q[1],q[2]
tdg q[1]; //cycle: 7 //tdg q[0]
cx q[0],q[1]; //cycle: 9 //cx q[1],q[0]
t q[2]; //cycle: 9 //t q[2]
tdg q[0]; //cycle: 11 //tdg q[1]
tdg q[1]; //cycle: 11 //tdg q[0]
cx q[2],q[1]; //cycle: 12 //cx q[2],q[0]
cx q[0],q[2]; //cycle: 14 //cx q[1],q[2]
cx q[1],q[0]; //cycle: 16 //cx q[0],q[1]
h q[2]; //cycle: 16 //h q[2]
t q[2]; //cycle: 17 //t q[2]
h q[1]; //cycle: 18 //h q[0]
t q[0]; //cycle: 18 //t q[1]
cx q[0],q[2]; //cycle: 19 //cx q[1],q[2]
t q[1]; //cycle: 19 //t q[0]
cx q[1],q[0]; //cycle: 21 //cx q[0],q[1]
cx q[2],q[1]; //cycle: 23 //cx q[2],q[0]
tdg q[0]; //cycle: 23 //tdg q[1]
cx q[2],q[0]; //cycle: 25 //cx q[2],q[1]
t q[1]; //cycle: 25 //t q[0]
tdg q[2]; //cycle: 27 //tdg q[2]
tdg q[0]; //cycle: 27 //tdg q[1]
cx q[1],q[0]; //cycle: 28 //cx q[0],q[1]
cx q[2],q[1]; //cycle: 30 //cx q[2],q[0]
cx q[0],q[2]; //cycle: 32 //cx q[1],q[2]
h q[1]; //cycle: 32 //h q[0]
t q[1]; //cycle: 33 //t q[0]
h q[2]; //cycle: 34 //h q[2]
t q[0]; //cycle: 34 //t q[1]
cx q[1],q[0]; //cycle: 35 //cx q[0],q[1]
t q[2]; //cycle: 35 //t q[2]
cx q[2],q[1]; //cycle: 37 //cx q[2],q[0]
cx q[0],q[2]; //cycle: 39 //cx q[1],q[2]
tdg q[1]; //cycle: 39 //tdg q[0]
cx q[0],q[1]; //cycle: 41 //cx q[1],q[0]
t q[2]; //cycle: 41 //t q[2]
tdg q[0]; //cycle: 43 //tdg q[1]
tdg q[1]; //cycle: 43 //tdg q[0]
cx q[2],q[1]; //cycle: 44 //cx q[2],q[0]
cx q[0],q[2]; //cycle: 46 //cx q[1],q[2]
cx q[1],q[0]; //cycle: 48 //cx q[0],q[1]
h q[2]; //cycle: 48 //h q[2]
cx q[2],q[0]; //cycle: 50 //cx q[2],q[1]
//50 original gates
//50 gates in generated circuit
//52 ideal depth (cycles)
//52 depth of generated circuit
//77 nodes popped from queue for processing.
//326 nodes remain in queue.
//HashFilter filtered 146 total nodes.
//HashFilter2 filtered 19 total nodes.
//HashFilter2 marked 15 total nodes.
