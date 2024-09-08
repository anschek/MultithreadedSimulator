#pragma once
#include <future>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>

enum TaskPriority {
	HIGH,
	NORMAL,
	LOW
};

template <typename Func, typename Arg>
class Task {
public:
	using ReturnType = std::invoke_result_t<Func, std::decay_t<Arg>>;

	Task(Func funPtr, Arg&& data, TaskPriority task_priority=NORMAL)
		:funPtr_(funPtr), data_(std::forward<Arg>(data)), task_priority_(task_priority),
		task_id_(++current_tasks_count), task_state_(UNSTARTED) {}
	
	Task(Func funPtr, TaskPriority task_priority=NORMAL)
		:funPtr_(funPtr), data_(std::nullopt), task_priority_(task_priority),
			task_id_(++current_tasks_count), task_state_(UNSTARTED) {}

	void Process() {
		try {
			task_state_ = IN_PROGRESS;
			ProcessAsyncFuture();
			task_state_ = COMPLETED;
		}
		catch (const std::exception& e) {
			task_state_ = ERROR;
			std::cerr << "Exception in Procces: " << e.what() << '\n';
		}
	}

	ReturnType GetResult() {
		ReturnType result;
		if (asyncFutureResult_.valid()) {
			try {
				result = asyncFutureResult_.get();
			}
			catch (const std::exception& e) {
				task_state_ = ERROR;
				std::cerr << "Exception in getting result: " << e.what() << '\n';
				result = ReturnType{}; 
			}
		}
		return result;
	}

	enum TaskState {
		UNSTARTED,
		IN_PROGRESS,
		COMPLETED,
		ERROR
	};

	TaskPriority GetTaskPriority() const {
		return task_priority_;
	}

	unsigned GetTaskId() const {
		return task_id_;
	}

	TaskState GetTaskState() const {
		return task_state_;
	}
	~Task()
	{
		if (asyncFutureResult_.valid()) {
			asyncFutureResult_.wait();
		} 
	}

private:
	std::optional<std::decay_t<Arg>> data_;
	Func funPtr_;
	std::future<ReturnType> asyncFutureResult_;

	TaskPriority task_priority_;
	unsigned task_id_;
	TaskState task_state_;

	static inline unsigned current_tasks_count = 0;

	void ProcessAsyncFuture() {
		if constexpr (std::is_same_v<Arg, void>) {
			asyncFutureResult_ = std::async(std::launch::async, funPtr_);
		}
		else {
			asyncFutureResult_ = std::async(std::launch::async, funPtr_, std::ref(*data_));
		}
	}
};

template <typename Func, typename Arg>
auto MakeTask(Func funcPtr, Arg&& data, TaskPriority task_priority=TaskPriority::NORMAL) {
	return Task<Func, std::decay_t<Arg>>(funcPtr, std::forward<Arg>(data), task_priority);
}

template <typename Func>
auto MakeTask(Func funcPtr, TaskPriority task_priority = TaskPriority::NORMAL) {
	return Task<Func, void>(funcPtr, task_priority);
}