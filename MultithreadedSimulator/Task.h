#pragma once
#include <future>
#include <functional>
#include <optional>
#include <thread>
#include <type_traits>
#include <utility>
#include <shared_mutex>

// generic template with arguments
template<typename Func, typename Arg>
struct ReturnTypeDeterminer {
	using type = std::invoke_result_t<Func, std::decay_t<Arg>>;
};

// template specialization without arguments
template<typename Func>
struct ReturnTypeDeterminer<Func, void> {
	using type = std::invoke_result_t<Func>;
};

enum TaskPriority {
	HIGH,
	NORMAL,
	LOW
};

enum TaskState {
	UNSTARTED,
	IN_PROGRESS,
	SUCCESS,
	ERROR
};

template <typename Func, typename Arg = void>
class Task {
public:
	// even with use conditional_t a specialization error occurs
	using ReturnType = typename ReturnTypeDeterminer<Func, Arg>::type;

	// constructor for func with arguments
	template<typename T = Arg, typename = std::enable_if_t<!std::is_same<T, void>::value>>
	Task(Func funPtr, T&& data, std::shared_mutex* mutex = nullptr, TaskPriority task_priority = NORMAL)
		: fun_ptr_(std::move(funPtr)), data_(std::forward<T>(data)), task_priority_(task_priority),
		task_id_(++current_tasks_count), task_state_(UNSTARTED), mutex_(mutex) {}

	template<typename T = Arg, typename = std::enable_if_t<std::is_same<T, void>::value>>
	Task(Func funPtr, TaskPriority task_priority = NORMAL)
		: fun_ptr_(std::move(funPtr)), data_(std::monostate{}), task_priority_(task_priority),
		task_id_(++current_tasks_count), task_state_(UNSTARTED), mutex_(nullptr){}

	void Process() {
		try {
			task_state_ = IN_PROGRESS;
			ProcessAsyncFuture();
		}
		catch (const std::exception& e) {
			task_state_ = ERROR;
			std::cerr << "Exception in Procces: " << e.what() << '\n';
		}
	}

	ReturnType GetResult() {
		try {
			task_state_ = SUCCESS;
			return async_future_result_.get();
		}
		catch (const std::exception& e) {
			task_state_ = ERROR;
			std::cerr << "Exception in getting result: " << e.what() << '\n';
		}
	}

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
		if (async_future_result_.valid()) {
			async_future_result_.wait();
		}
	}

private:
	// if don't use this construct, the code will not compile because of the possibility of std::decay_t<void>
	using ArgType = std::conditional_t<std::is_same_v<Arg, void>,	// condition
		std::monostate, std::decay_t<Arg>>;						   // if  else

	ArgType data_; // instead of std::optional<std::decay_t<Arg>> data_
	Func fun_ptr_;
	std::future<ReturnType> async_future_result_;

	TaskPriority task_priority_;
	unsigned task_id_;
	TaskState task_state_;

	// inline includes a field from all files only once
	static inline unsigned current_tasks_count = 0;

	void ProcessAsyncFuture() {
		// constexpr compiles only the part of the branching that is true
		if constexpr (!std::is_same_v<Arg, void>) {
			//Arg (lvalue): without mutex
			if (!std::is_reference_v<Arg>) {
				async_future_result_ = std::async(std::launch::async, fun_ptr_, data_);
			}//Arg&: write mutex			
			else if (!std::is_const_v<std::remove_reference_t<Arg>>) {
				std::unique_lock lock(*mutex_);
				async_future_result_ = std::async(std::launch::async, fun_ptr_, data_);
			}//const Arg&: read mutex
			else {
				std::shared_lock lock(*mutex_);
				async_future_result_ = std::async(std::launch::async, fun_ptr_, data_);
			}
		}//void: without mutex
		else {
			async_future_result_ = std::async(std::launch::async, fun_ptr_);
		}
	}

	//std::shared_mutex& mutex_;
	std::shared_mutex* mutex_;
};

template <typename Func, typename Arg>
auto MakeTask(Func funcPtr, Arg&& data, std::shared_mutex* mutex = nullptr, TaskPriority task_priority = TaskPriority::NORMAL) {
	return Task<Func, std::decay_t<Arg>>(std::move(funcPtr), std::forward<Arg>(data), mutex, task_priority);
}

template <typename Func>
auto MakeTask(Func funcPtr, TaskPriority task_priority = TaskPriority::NORMAL) {
	return Task<Func>(std::move(funcPtr), task_priority);
}