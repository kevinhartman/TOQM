#ifndef LINKEDSTACK_HPP
#define LINKEDSTACK_HPP

namespace toqm {

template<class T>
class LinkedStack {
public:
	T value;
	std::shared_ptr<LinkedStack<T>> next;
	int size;
	
	LinkedStack() {
		this->value = NULL;
		this->size = 0;
		this->next = NULL;
	}
	
	//Create a new LinkedStack such that this one is its second element
	//Returns the new LinkedStack
	static std::unique_ptr<LinkedStack<T>> push(const std::shared_ptr<LinkedStack<T>>& head, T newVal) {
		auto ret = std::unique_ptr<LinkedStack<T>>(new LinkedStack<T>);
		ret->next = head;
		ret->size = head->size + 1;
		ret->value = newVal;
		return ret;
	}
};

}

#endif
