#pragma once
#include <future>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>

//TODO
//add getters to priority, state, id
//modify state in process
//add try/catch on process/get result/destructor
//split to files
//unit tests


enum TaskPriority {
	HIGH,
	NORMAL,
	LOW
};

template <typename Func, typename Arg>
class Task {
public:
	using ReturnType = std::invoke_result_t<Func, std::decay_t<Arg>>;

	Task(Func funPtr, Arg&& data, TaskPriority task_priority)
		:funPtr_(funPtr), data_(std::forward<Arg>(data)), task_priority_(task_priority),
		task_id_(++current_tasks_count), task_state_(UNSTARTED) {}
	
	Task(Func funPtr, TaskPriority task_priority)
		:funPtr_(funPtr), data_(std::nullopt), task_priority_(task_priority),
			task_id_(++current_tasks_count), task_state_(UNSTARTED) {}

	Task(Func funPtr, Arg&& data)
		: Task(funPtr, std::forward<Arg>(data), NORMAL) {}

	Task(Func funPtr)
		:funPtr_(funPtr, NORMAL) {}

	template <typename T = Arg>
	std::enable_if_t<!std::is_same_v<T, void>, void>
		Process() {
		asyncFutureResult_ = std::async(std::launch::async, funPtr_, std::ref(*data_));
	}

	template <typename T = Arg>
	std::enable_if_t<std::is_same_v<T, void>, void>
		Process() {
		asyncFutureResult_ = std::async(std::launch::async, funPtr_);
	}

	ReturnType GetResult() {
		if (asyncFutureResult_.valid()) {
			return asyncFutureResult_.get();
		}else {
			return ReturnType{};
		}
	}
	//get/set on private fields (parameters): id, state, status (read only)

	~Task()
	{
		if (asyncFutureResult_.valid()) {
			asyncFutureResult_.get();
		} 
	}

	enum TaskState {
		UNSTARTED,
		IN_PROGRESS,
		COMPLETED,
		ERROR
	};

private:
	std::optional<std::decay_t<Arg>> data_;
	Func funPtr_;
	std::future<ReturnType> asyncFutureResult_;

	TaskPriority task_priority_;
	unsigned task_id_;
	TaskState task_state_;

	static inline unsigned current_tasks_count = 0;
};

template <typename Func, typename Arg>
Task<Func, Arg> MakeTask(Func funcPtr, Arg&& data, TaskPriority task_priority) {
	return Task<Func, Arg>(funcPtr, std::forward<Arg>(data), task_priority);
}

template <typename Func>
Task<Func, void> MakeTask(Func funcPtr, TaskPriority task_priority) {
	return Task<Func, void>(funcPtr, task_priority);
}

template <typename Func, typename Arg>
Task<Func, Arg> MakeTask(Func funcPtr, Arg&& data) {
	return Task<Func, Arg>(funcPtr, std::forward<Arg>(data));
}

template <typename Func>
Task<Func, void> MakeTask(Func funcPtr) {
	return Task<Func, void>(funcPtr);
}