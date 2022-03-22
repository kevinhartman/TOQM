//Note: initial mapping (logical qubit at each location): 2, 1, 4, 3, 0, 
//Note: initial mapping (location of each logical qubit): 4, 1, 0, 3, 2, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
swap q[3],q[4]; //cycle: 0
h q[1]; //cycle: 0 //h q[1]
t q[2]; //cycle: 0 //t q[4]
t q[0]; //cycle: 0 //t q[2]
cx q[0],q[2]; //cycle: 1 //cx q[2],q[4]
t q[1]; //cycle: 1 //t q[1]
cx q[1],q[0]; //cycle: 3 //cx q[1],q[2]
cx q[2],q[1]; //cycle: 5 //cx q[4],q[1]
tdg q[0]; //cycle: 5 //tdg q[2]
cx q[2],q[0]; //cycle: 7 //cx q[4],q[2]
t q[1]; //cycle: 7 //t q[1]
tdg q[2]; //cycle: 9 //tdg q[4]
tdg q[0]; //cycle: 9 //tdg q[2]
cx q[1],q[0]; //cycle: 10 //cx q[1],q[2]
cx q[2],q[1]; //cycle: 12 //cx q[4],q[1]
cx q[0],q[2]; //cycle: 14 //cx q[2],q[4]
h q[1]; //cycle: 14 //h q[1]
t q[1]; //cycle: 15 //t q[1]
cx q[2],q[4]; //cycle: 16 //cx q[4],q[3]
x q[0]; //cycle: 16 //x q[2]
h q[0]; //cycle: 17 //h q[2]
t q[2]; //cycle: 18 //t q[4]
t q[0]; //cycle: 18 //t q[2]
t q[4]; //cycle: 18 //t q[3]
cx q[2],q[1]; //cycle: 19 //cx q[4],q[1]
cx q[0],q[2]; //cycle: 21 //cx q[2],q[4]
cx q[1],q[0]; //cycle: 23 //cx q[1],q[2]
tdg q[2]; //cycle: 23 //tdg q[4]
cx q[1],q[2]; //cycle: 25 //cx q[1],q[4]
t q[0]; //cycle: 25 //t q[2]
tdg q[1]; //cycle: 27 //tdg q[1]
tdg q[2]; //cycle: 27 //tdg q[4]
cx q[0],q[2]; //cycle: 28 //cx q[2],q[4]
cx q[1],q[0]; //cycle: 30 //cx q[1],q[2]
swap q[0],q[2]; //cycle: 32
swap q[0],q[1]; //cycle: 38
h q[2]; //cycle: 38 //h q[2]
cx q[2],q[3]; //cycle: 39 //cx q[2],q[0]
h q[2]; //cycle: 41 //h q[2]
t q[3]; //cycle: 41 //t q[0]
cx q[4],q[3]; //cycle: 42 //cx q[3],q[0]
t q[2]; //cycle: 42 //t q[2]
cx q[1],q[0]; //cycle: 44 //cx q[4],q[1]
cx q[2],q[4]; //cycle: 44 //cx q[2],q[3]
cx q[3],q[2]; //cycle: 46 //cx q[0],q[2]
tdg q[4]; //cycle: 46 //tdg q[3]
cx q[3],q[4]; //cycle: 48 //cx q[0],q[3]
t q[2]; //cycle: 48 //t q[2]
tdg q[3]; //cycle: 50 //tdg q[0]
tdg q[4]; //cycle: 50 //tdg q[3]
cx q[2],q[4]; //cycle: 51 //cx q[2],q[3]
cx q[3],q[2]; //cycle: 53 //cx q[0],q[2]
cx q[4],q[3]; //cycle: 55 //cx q[3],q[0]
h q[2]; //cycle: 55 //h q[2]
cx q[2],q[4]; //cycle: 57 //cx q[2],q[3]
//52 original gates
//55 gates in generated circuit
//53 ideal depth (cycles)
//59 depth of generated circuit
//2428 nodes popped from queue for processing.
//3133 nodes remain in queue.
//HashFilter filtered 2882 total nodes.
//HashFilter2 filtered 1376 total nodes.
//HashFilter2 marked 2146 total nodes.
