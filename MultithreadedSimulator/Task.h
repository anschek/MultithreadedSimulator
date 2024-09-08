#pragma once
#include <thread>
#include <future>
#include <utility>
#include <type_traits>
#include <optional>

//TODO
//add logging
//exception handling
//make it possible to get result when calling Process with async execute (now it returns not result T, but obj future<T>)

template <typename Func, typename Arg>
class Task {
public:
	using ReturnType = std::invoke_result_t<Func, std::decay_t<Arg>>;

	Task(Func funPtr, Arg&& data)
		:funPtr_(funPtr), data_(std::forward<Arg>(data)) {}
	Task(Func funPtr)
		:funPtr_(nullptr), data_(std::nullopt) {}

	template <typename T = Arg>
	std::enable_if_t<!std::is_same_v<T, void>, std::future<ReturnType>>
		Process() {
		return std::async(std::launch::async, funPtr_, std::ref(*data_));
	}
	template <typename T = Arg>
	std::enable_if_t<std::is_same_v<T, void>, std::future<ReturnType>>
		Process() {
		return std::async(std::launch::async, funPtr_);
	}
	//get/set on private fields (parameters)

	//~Task()
	//{
	//	if (asyncFutureResult_.valid()) {
	//		asyncFutureResult_.get();
	//	} 
	//}
private:
	std::optional<std::decay_t<Arg>> data_;
	Func funPtr_;
	//std::future<ReturnType> asyncFutureResult_;
	
	//parameters: 
	//task priority, 
	//max execution time, 
	//id, 
	//task state/status (in progress, completed, error, etc.)
};

template <typename Func, typename Arg>
Task<Func, Arg> MakeTask(Func funcPtr, Arg&& data) {
	return Task<Func, Arg>(funcPtr, std::forward<Arg>(data));
}

template <typename Func>
Task<Func, void> MakeTask(Func funcPtr) {
	return Task<Func, void>(funcPtr);
}