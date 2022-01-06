#ifndef QASMOBJECT_HPP
#define QASMOBJECT_HPP

#include "CommonTypes.hpp"

#include <iosfwd>
#include <memory>

namespace toqm {

class ToqmResult;

class QasmObject {
public:
	QasmObject();
	
	~QasmObject();
	
	const std::vector<GateOp> & gateOperations() const;
	
	std::size_t numQubits() const;
	
	void toQasm2(std::ostream & out, const ToqmResult & result) const;
	
	/**
	 * Parses a quantum file.
	 * @param in stream of openqasm 2.0 contents
	 * @return an object representation of the openqasm 2.0 contents.
	 */
	static std::unique_ptr<QasmObject> fromQasm2(std::istream & in);

private:
	class Impl;
	
	std::unique_ptr<Impl> impl;
};

constexpr auto parseQasm2 = QasmObject::fromQasm2;

}

#endif
