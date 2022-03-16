//Note: initial mapping (logical qubit at each location): 3, 1, 0, -1, 2, 
//Note: initial mapping (location of each logical qubit): 2, 1, 4, 0, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
h q[0]; //cycle: 0 //h q[3]
t q[1]; //cycle: 0 //t q[1]
t q[2]; //cycle: 0 //t q[0]
t q[4]; //cycle: 0 //t q[2]
cx q[2],q[1]; //cycle: 1 //cx q[0],q[1]
t q[0]; //cycle: 1 //t q[3]
cx q[0],q[2]; //cycle: 3 //cx q[3],q[0]
cx q[1],q[0]; //cycle: 5 //cx q[1],q[3]
tdg q[2]; //cycle: 5 //tdg q[0]
cx q[1],q[2]; //cycle: 7 //cx q[1],q[0]
t q[0]; //cycle: 7 //t q[3]
tdg q[1]; //cycle: 9 //tdg q[1]
tdg q[2]; //cycle: 9 //tdg q[0]
cx q[0],q[2]; //cycle: 10 //cx q[3],q[0]
cx q[1],q[0]; //cycle: 12 //cx q[1],q[3]
cx q[2],q[1]; //cycle: 14 //cx q[0],q[1]
h q[0]; //cycle: 14 //h q[3]
h q[0]; //cycle: 15 //h q[3]
cx q[2],q[1]; //cycle: 16 //cx q[0],q[1]
t q[0]; //cycle: 16 //t q[3]
swap q[2],q[4]; //cycle: 18
t q[1]; //cycle: 18 //t q[1]
cx q[1],q[2]; //cycle: 24 //cx q[1],q[2]
cx q[0],q[1]; //cycle: 26 //cx q[3],q[1]
cx q[2],q[0]; //cycle: 28 //cx q[2],q[3]
tdg q[1]; //cycle: 28 //tdg q[1]
cx q[2],q[1]; //cycle: 30 //cx q[2],q[1]
t q[0]; //cycle: 30 //t q[3]
tdg q[2]; //cycle: 32 //tdg q[2]
tdg q[1]; //cycle: 32 //tdg q[1]
cx q[0],q[1]; //cycle: 33 //cx q[3],q[1]
cx q[2],q[0]; //cycle: 35 //cx q[2],q[3]
cx q[1],q[2]; //cycle: 37 //cx q[1],q[2]
h q[0]; //cycle: 37 //h q[3]
cx q[1],q[2]; //cycle: 39 //cx q[1],q[2]
//34 original gates
//35 gates in generated circuit
//36 ideal depth (cycles)
//41 depth of generated circuit
//890 nodes popped from queue for processing.
//1213 nodes remain in queue.
//HashFilter filtered 1839 total nodes.
//HashFilter2 filtered 673 total nodes.
//HashFilter2 marked 820 total nodes.
