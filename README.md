## libtoqm

A C++11 class library port of the
[reference implementation](https://github.com/time-optimal-qmapper/TOQM/tree/main/code) originally published alongside
paper [Time-Optimal Qubit Mapping](https://doi.org/10.1145/3445814.3446706).

There are two build targets here:
- `libtoqm`, a class library that exposes the full breadth of configuration options and algorithms
  available in the reference implementation.
- `mapper`, an interactive commandline program that mimics the behavior of the original reference
  program, written on top of `libtoqm`.

### Example usage
```cpp
// Construct mapper with desired configuration.
toqm::ToqmMapper m(
	toqm::DefaultQueue {},
	std::make_unique<toqm::DefaultExpander>(),
	std::make_unique<toqm::CXFrontier>(),
	std::make_unique<toqm::Latency_1_3>(),
	{},
	{std::make_unique<toqm::HashFilter>(), std::make_unique<toqm::HashFilter2>()}
);

// Run TOQM algorithm on a particular gate listing and target.
m.run(gates, num_qubits, coupling_map);
```

### References
- [Time-Optimal Qubit Mapping](https://doi.org/10.1145/3445814.3446706) paper:
  Chi Zhang, Ari B. Hayes, Longfei Qiu, Yuwei Jin, Yanhao Chen, and Eddy Z. Zhang. 2021. Time-Optimal Qubit
  Mapping. In Proceedings of the 26th ACM International Conference on Architectural Support for Programming
  Languages and Operating Systems (ASPLOS ’21), April 19–23, 2021, Virtual, USA.
  ACM, New York, NY, USA, 14 pages.
- [TOQM reference implementation](https://github.com/time-optimal-qmapper/TOQM/tree/main/code), published with paper.
- @arihayes [fork](https://github.com/arihayes/TOQM), which adds the `Table` latency class used in this library to
  support arbitrary 2 qubit operations and also make changes to how latency is handled.