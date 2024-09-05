#pragma once

//TODO
//add logging
//exception handling

template <typename T, typename ProccessedData>
class Task {
public:
	Task(T(*funPtr)()) {

	}	
	Task(T(*funPtr)(ProccessedData), ProccessedData& data) {

	}
	//process() - data processing
	// return<T> fun(data)
	T Process() {

	}

	//get/set on private fields (parameters)
private:
	ProccessedData& data_;
	T (*funcPtr_)(ProccessedData);
	T (*funcPtrVoid_)();

	//parameters: 
	//task priority, 
	//max execution time, 
	//id, 
	//task state/status (in progress, completed, error, etc.)
};