//Note: initial mapping (logical qubit at each location): 1, 2, 4, 0, 3, 
//Note: initial mapping (location of each logical qubit): 3, 0, 1, 4, 2, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[4]; //cycle: 0 //cx q[4],q[3]
cx q[1],q[0]; //cycle: 0 //cx q[2],q[1]
x q[1]; //cycle: 2 //x q[2]
t q[2]; //cycle: 2 //t q[4]
t q[0]; //cycle: 2 //t q[1]
t q[4]; //cycle: 2 //t q[3]
cx q[0],q[2]; //cycle: 3 //cx q[1],q[4]
h q[1]; //cycle: 3 //h q[2]
t q[1]; //cycle: 4 //t q[2]
cx q[1],q[0]; //cycle: 5 //cx q[2],q[1]
cx q[2],q[1]; //cycle: 7 //cx q[4],q[2]
tdg q[0]; //cycle: 7 //tdg q[1]
cx q[2],q[0]; //cycle: 9 //cx q[4],q[1]
t q[1]; //cycle: 9 //t q[2]
swap q[1],q[2]; //cycle: 11
tdg q[0]; //cycle: 11 //tdg q[1]
cx q[2],q[0]; //cycle: 17 //cx q[2],q[1]
tdg q[1]; //cycle: 17 //tdg q[4]
cx q[1],q[2]; //cycle: 19 //cx q[4],q[2]
swap q[0],q[1]; //cycle: 21
h q[2]; //cycle: 21 //h q[2]
cx q[2],q[3]; //cycle: 22 //cx q[2],q[0]
h q[2]; //cycle: 24 //h q[2]
t q[3]; //cycle: 24 //t q[0]
cx q[3],q[4]; //cycle: 25 //cx q[0],q[3]
t q[2]; //cycle: 25 //t q[2]
cx q[1],q[0]; //cycle: 27 //cx q[1],q[4]
cx q[2],q[3]; //cycle: 27 //cx q[2],q[0]
cx q[4],q[2]; //cycle: 29 //cx q[3],q[2]
tdg q[3]; //cycle: 29 //tdg q[0]
cx q[4],q[3]; //cycle: 31 //cx q[3],q[0]
t q[2]; //cycle: 31 //t q[2]
tdg q[4]; //cycle: 33 //tdg q[3]
tdg q[3]; //cycle: 33 //tdg q[0]
cx q[2],q[3]; //cycle: 34 //cx q[2],q[0]
cx q[4],q[2]; //cycle: 36 //cx q[3],q[2]
cx q[3],q[4]; //cycle: 38 //cx q[0],q[3]
h q[2]; //cycle: 38 //h q[2]
cx q[2],q[4]; //cycle: 40 //cx q[2],q[3]
//37 original gates
//39 gates in generated circuit
//37 ideal depth (cycles)
//42 depth of generated circuit
//452 nodes popped from queue for processing.
//815 nodes remain in queue.
//HashFilter filtered 2165 total nodes.
//HashFilter2 filtered 234 total nodes.
//HashFilter2 marked 301 total nodes.
