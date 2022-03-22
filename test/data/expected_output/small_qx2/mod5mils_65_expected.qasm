//Note: initial mapping (logical qubit at each location): 2, 0, 1, 3, 4, 
//Note: initial mapping (location of each logical qubit): 1, 2, 0, 3, 4, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[3]; //cycle: 0 //cx q[1],q[3]
h q[4]; //cycle: 0 //h q[4]
t q[1]; //cycle: 0 //t q[0]
t q[0]; //cycle: 0 //t q[2]
t q[4]; //cycle: 1 //t q[4]
swap q[1],q[2]; //cycle: 2
x q[3]; //cycle: 2 //x q[3]
t q[3]; //cycle: 3 //t q[3]
cx q[2],q[3]; //cycle: 8 //cx q[0],q[3]
swap q[0],q[1]; //cycle: 8
cx q[4],q[2]; //cycle: 10 //cx q[4],q[0]
cx q[3],q[4]; //cycle: 12 //cx q[3],q[4]
tdg q[2]; //cycle: 12 //tdg q[0]
cx q[3],q[2]; //cycle: 14 //cx q[3],q[0]
t q[4]; //cycle: 14 //t q[4]
tdg q[3]; //cycle: 16 //tdg q[3]
tdg q[2]; //cycle: 16 //tdg q[0]
cx q[4],q[2]; //cycle: 17 //cx q[4],q[0]
cx q[3],q[4]; //cycle: 19 //cx q[3],q[4]
cx q[2],q[3]; //cycle: 21 //cx q[0],q[3]
h q[4]; //cycle: 21 //h q[4]
h q[4]; //cycle: 22 //h q[4]
swap q[1],q[2]; //cycle: 23
t q[3]; //cycle: 23 //t q[3]
t q[4]; //cycle: 23 //t q[4]
cx q[2],q[3]; //cycle: 29 //cx q[2],q[3]
cx q[4],q[2]; //cycle: 31 //cx q[4],q[2]
cx q[3],q[4]; //cycle: 33 //cx q[3],q[4]
tdg q[2]; //cycle: 33 //tdg q[2]
cx q[3],q[2]; //cycle: 35 //cx q[3],q[2]
t q[4]; //cycle: 35 //t q[4]
tdg q[3]; //cycle: 37 //tdg q[3]
tdg q[2]; //cycle: 37 //tdg q[2]
cx q[4],q[2]; //cycle: 38 //cx q[4],q[2]
cx q[3],q[4]; //cycle: 40 //cx q[3],q[4]
cx q[2],q[3]; //cycle: 42 //cx q[2],q[3]
h q[4]; //cycle: 42 //h q[4]
cx q[3],q[4]; //cycle: 44 //cx q[3],q[4]
//35 original gates
//38 gates in generated circuit
//37 ideal depth (cycles)
//46 depth of generated circuit
//4965 nodes popped from queue for processing.
//1598 nodes remain in queue.
//HashFilter filtered 5438 total nodes.
//HashFilter2 filtered 2288 total nodes.
//HashFilter2 marked 3111 total nodes.
