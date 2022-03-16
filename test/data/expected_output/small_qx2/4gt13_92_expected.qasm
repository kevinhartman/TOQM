//Note: initial mapping (logical qubit at each location): 3, 2, 4, 0, 1, 
//Note: initial mapping (location of each logical qubit): 3, 4, 1, 0, 2, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[3]; //cycle: 0 //cx q[4],q[0]
t q[4]; //cycle: 0 //t q[1]
t q[0]; //cycle: 0 //t q[3]
t q[1]; //cycle: 0 //t q[2]
cx q[1],q[0]; //cycle: 1 //cx q[2],q[3]
h q[3]; //cycle: 2 //h q[0]
t q[2]; //cycle: 2 //t q[4]
cx q[2],q[4]; //cycle: 3 //cx q[4],q[1]
t q[3]; //cycle: 3 //t q[0]
cx q[3],q[2]; //cycle: 5 //cx q[0],q[4]
cx q[4],q[3]; //cycle: 7 //cx q[1],q[0]
tdg q[2]; //cycle: 7 //tdg q[4]
cx q[4],q[2]; //cycle: 9 //cx q[1],q[4]
t q[3]; //cycle: 9 //t q[0]
tdg q[4]; //cycle: 11 //tdg q[1]
tdg q[2]; //cycle: 11 //tdg q[4]
cx q[3],q[2]; //cycle: 12 //cx q[0],q[4]
cx q[4],q[3]; //cycle: 14 //cx q[1],q[0]
cx q[2],q[4]; //cycle: 16 //cx q[4],q[1]
h q[3]; //cycle: 16 //h q[0]
h q[3]; //cycle: 17 //h q[0]
h q[2]; //cycle: 18 //h q[4]
t q[4]; //cycle: 18 //t q[1]
t q[3]; //cycle: 18 //t q[0]
t q[2]; //cycle: 19 //t q[4]
cx q[2],q[1]; //cycle: 20 //cx q[4],q[2]
cx q[0],q[2]; //cycle: 22 //cx q[3],q[4]
tdg q[1]; //cycle: 22 //tdg q[2]
cx q[0],q[1]; //cycle: 24 //cx q[3],q[2]
t q[2]; //cycle: 24 //t q[4]
tdg q[0]; //cycle: 26 //tdg q[3]
tdg q[1]; //cycle: 26 //tdg q[2]
cx q[2],q[1]; //cycle: 27 //cx q[4],q[2]
cx q[0],q[2]; //cycle: 29 //cx q[3],q[4]
swap q[0],q[1]; //cycle: 31
h q[2]; //cycle: 31 //h q[4]
t q[2]; //cycle: 32 //t q[4]
cx q[2],q[4]; //cycle: 33 //cx q[4],q[1]
cx q[3],q[2]; //cycle: 35 //cx q[0],q[4]
cx q[0],q[1]; //cycle: 37 //cx q[2],q[3]
cx q[4],q[3]; //cycle: 37 //cx q[1],q[0]
tdg q[2]; //cycle: 37 //tdg q[4]
cx q[4],q[2]; //cycle: 39 //cx q[1],q[4]
t q[3]; //cycle: 39 //t q[0]
t q[1]; //cycle: 39 //t q[3]
t q[0]; //cycle: 39 //t q[2]
cx q[0],q[1]; //cycle: 40 //cx q[2],q[3]
tdg q[4]; //cycle: 41 //tdg q[1]
tdg q[2]; //cycle: 41 //tdg q[4]
cx q[3],q[2]; //cycle: 42 //cx q[0],q[4]
cx q[4],q[3]; //cycle: 44 //cx q[1],q[0]
cx q[2],q[4]; //cycle: 46 //cx q[4],q[1]
h q[3]; //cycle: 46 //h q[0]
h q[2]; //cycle: 48 //h q[4]
t q[2]; //cycle: 49 //t q[4]
cx q[2],q[0]; //cycle: 50 //cx q[4],q[2]
cx q[1],q[2]; //cycle: 52 //cx q[3],q[4]
tdg q[0]; //cycle: 52 //tdg q[2]
cx q[1],q[0]; //cycle: 54 //cx q[3],q[2]
t q[2]; //cycle: 54 //t q[4]
tdg q[1]; //cycle: 56 //tdg q[3]
tdg q[0]; //cycle: 56 //tdg q[2]
cx q[2],q[0]; //cycle: 57 //cx q[4],q[2]
cx q[1],q[2]; //cycle: 59 //cx q[3],q[4]
cx q[0],q[1]; //cycle: 61 //cx q[2],q[3]
h q[2]; //cycle: 61 //h q[4]
cx q[3],q[2]; //cycle: 62 //cx q[0],q[4]
//66 original gates
//67 gates in generated circuit
//64 ideal depth (cycles)
//64 depth of generated circuit
//454 nodes popped from queue for processing.
//1500 nodes remain in queue.
//HashFilter filtered 686 total nodes.
//HashFilter2 filtered 138 total nodes.
//HashFilter2 marked 226 total nodes.
