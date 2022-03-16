//Note: initial mapping (logical qubit at each location): 2, 1, 3, 4, 0, 
//Note: initial mapping (location of each logical qubit): 4, 1, 0, 2, 3, 
OPENQASM 2.0;
include "qelib1.inc";
qreg q[5];
creg c[5];
cx q[2],q[3]; //cycle: 0 //cx q[3],q[4]
cx q[1],q[0]; //cycle: 0 //cx q[1],q[2]
x q[2]; //cycle: 2 //x q[3]
h q[1]; //cycle: 2 //h q[1]
t q[0]; //cycle: 2 //t q[2]
t q[3]; //cycle: 2 //t q[4]
swp q[3],q[4]; //cycle: 3
t q[2]; //cycle: 3 //t q[3]
t q[1]; //cycle: 3 //t q[1]
cx q[0],q[2]; //cycle: 4 //cx q[2],q[3]
cx q[1],q[0]; //cycle: 6 //cx q[1],q[2]
cx q[2],q[1]; //cycle: 8 //cx q[3],q[1]
tdg q[0]; //cycle: 8 //tdg q[2]
cx q[2],q[0]; //cycle: 10 //cx q[3],q[2]
t q[1]; //cycle: 10 //t q[1]
swp q[1],q[2]; //cycle: 12
tdg q[0]; //cycle: 12 //tdg q[2]
cx q[2],q[0]; //cycle: 18 //cx q[1],q[2]
tdg q[1]; //cycle: 18 //tdg q[3]
cx q[1],q[2]; //cycle: 20 //cx q[3],q[1]
swp q[0],q[1]; //cycle: 22
h q[2]; //cycle: 22 //h q[1]
cx q[2],q[3]; //cycle: 23 //cx q[1],q[0]
x q[2]; //cycle: 25 //x q[1]
t q[3]; //cycle: 25 //t q[0]
cx q[3],q[4]; //cycle: 26 //cx q[0],q[4]
h q[2]; //cycle: 26 //h q[1]
t q[2]; //cycle: 27 //t q[1]
cx q[1],q[0]; //cycle: 28 //cx q[2],q[3]
cx q[2],q[3]; //cycle: 28 //cx q[1],q[0]
cx q[4],q[2]; //cycle: 30 //cx q[4],q[1]
tdg q[3]; //cycle: 30 //tdg q[0]
cx q[4],q[3]; //cycle: 32 //cx q[4],q[0]
t q[2]; //cycle: 32 //t q[1]
tdg q[4]; //cycle: 34 //tdg q[4]
tdg q[3]; //cycle: 34 //tdg q[0]
cx q[2],q[3]; //cycle: 35 //cx q[1],q[0]
cx q[4],q[2]; //cycle: 37 //cx q[4],q[1]
cx q[3],q[4]; //cycle: 39 //cx q[0],q[4]
h q[2]; //cycle: 39 //h q[1]
//37 original gates
//40 gates in generated circuit
//36 ideal depth (cycles)
//41 depth of generated circuit
//673 nodes popped from queue for processing.
//1138 nodes remain in queue.
//HashFilter filtered 2255 total nodes.
//HashFilter2 filtered 395 total nodes.
//HashFilter2 marked 692 total nodes.
