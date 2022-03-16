//Note: initial mapping (logical qubit at each location): 0, 2, 1, 3, 4, 
//Note: initial mapping (location of each logical qubit): 0, 2, 1, 3, 4, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
x q[3]; //cycle: 0 //x q[3]
t q[1]; //cycle: 0 //t q[2]
t q[0]; //cycle: 0 //t q[0]
cx q[2],q[3]; //cycle: 1 //cx q[1],q[3]
cx q[3],q[4]; //cycle: 3 //cx q[3],q[4]
swap q[1],q[2]; //cycle: 3
h q[4]; //cycle: 5 //h q[4]
t q[3]; //cycle: 5 //t q[3]
t q[4]; //cycle: 6 //t q[4]
cx q[2],q[3]; //cycle: 9 //cx q[2],q[3]
swap q[0],q[1]; //cycle: 9
cx q[4],q[2]; //cycle: 11 //cx q[4],q[2]
cx q[3],q[4]; //cycle: 13 //cx q[3],q[4]
tdg q[2]; //cycle: 13 //tdg q[2]
cx q[3],q[2]; //cycle: 15 //cx q[3],q[2]
t q[4]; //cycle: 15 //t q[4]
tdg q[3]; //cycle: 17 //tdg q[3]
tdg q[2]; //cycle: 17 //tdg q[2]
cx q[4],q[2]; //cycle: 18 //cx q[4],q[2]
cx q[3],q[4]; //cycle: 20 //cx q[3],q[4]
cx q[2],q[3]; //cycle: 22 //cx q[2],q[3]
h q[4]; //cycle: 22 //h q[4]
h q[4]; //cycle: 23 //h q[4]
swap q[1],q[2]; //cycle: 24
t q[3]; //cycle: 24 //t q[3]
t q[4]; //cycle: 24 //t q[4]
cx q[2],q[3]; //cycle: 30 //cx q[0],q[3]
cx q[4],q[2]; //cycle: 32 //cx q[4],q[0]
cx q[3],q[4]; //cycle: 34 //cx q[3],q[4]
tdg q[2]; //cycle: 34 //tdg q[0]
cx q[3],q[2]; //cycle: 36 //cx q[3],q[0]
t q[4]; //cycle: 36 //t q[4]
tdg q[3]; //cycle: 38 //tdg q[3]
tdg q[2]; //cycle: 38 //tdg q[0]
cx q[4],q[2]; //cycle: 39 //cx q[4],q[0]
cx q[3],q[4]; //cycle: 41 //cx q[3],q[4]
cx q[2],q[3]; //cycle: 43 //cx q[0],q[3]
h q[4]; //cycle: 43 //h q[4]
//35 original gates
//38 gates in generated circuit
//37 ideal depth (cycles)
//45 depth of generated circuit
//4738 nodes popped from queue for processing.
//1803 nodes remain in queue.
//HashFilter filtered 5499 total nodes.
//HashFilter2 filtered 1899 total nodes.
//HashFilter2 marked 2632 total nodes.
