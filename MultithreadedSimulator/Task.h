#pragma once
#include <thread>
#include <future>
#include <utility>
#include <type_traits>

//TODO
//add logging
//exception handling

template <typename Func, typename Arg>
class Task {
public:
	using ReturnType = std::invoke_result_t<Func, Arg>;

	Task(Func funPtr, Arg&& data)
		:funPtr_(funPtr), funPtrVoid_(nullptr), data_(std::forward<Arg>(data)) {}
	Task(Func funPtr)
		:funPtrVoid_(funPtr), funPtr_(nullptr) {}

	std::future<ReturnType> Process() {
		asyncFutureResult_ = std::async(std::launch::async, funPtr_, std::ref(data_));
			//(funPtrVoid_ == nullptr)
			//? std::async(std::launch::async, funPtr_, std::ref(data_))
			//: std::async(std::launch::async, funPtrVoid_ );
		return std::move(asyncFutureResult_);
	}

	//get/set on private fields (parameters)
	~Task()
	{
		if (asyncFutureResult_.valid()) asyncFutureResult_.get();
	}
private:
	std::decay_t<Arg> data_;
	Func funPtr_;
	Func funPtrVoid_;
	std::future<ReturnType> asyncFutureResult_;
	
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



template <typename Func, typename Arg>
class Task1 {
public:
    using ReturnType = std::invoke_result_t<Func, Arg>;

    Task1(Func funPtr, Arg&& data)
        : funcPtr_(funPtr), data_(std::forward<Arg>(data)) {}

    ReturnType Process() {
        std::future<ReturnType> asyncFutureResult = std::async(std::launch::async, funcPtr_, std::ref(data_));
        return asyncFutureResult.get();//!!!
    }

    ~Task1() {
        if (curThread_.joinable()) curThread_.join();
    }

private:
    Func funcPtr_;
    std::decay_t<Arg> data_;
    std::thread curThread_;
};

// Фабричная функция для автоматического определения типов
template <typename Func, typename Arg>
auto MakeTask1(Func func, Arg&& data) {
    return Task1<Func, Arg>(func, std::forward<Arg>(data));
}
