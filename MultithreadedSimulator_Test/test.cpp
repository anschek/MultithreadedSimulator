#include "pch.h"
#include "gtest/gtest.h" //Google Test
#include "../MultithreadedSimulator/Task.h"

#include<numeric>
#include <type_traits>

namespace TaskTest {
    using namespace ::testing;

    auto sleeping_time = std::chrono::milliseconds(10);
    
    int counter_of_threads_samples = 10;

    class TaskFixture : public Test {
    protected:
        TaskFixture() : mutex_() {}

        std::shared_mutex mutex_;
    };

    int TaskFuncWithArg(int data) {
        std::this_thread::sleep_for(sleeping_time);
        return data + 10;
    }

    void TaskFuncWithRefArg(int& data) {
        std::this_thread::sleep_for(sleeping_time);
        data += 10;
    }

    int TaskFuncWithConstRefArg(const int& data) {
        std::this_thread::sleep_for(sleeping_time);
        return data * 3;
    }

    void TaskFuncWithoutArg() {
        std::this_thread::sleep_for(sleeping_time);
    }

    void TaskFuncException() {
        throw std::runtime_error("Test exception");
    }

    TEST_F(TaskFixture, GetResult_GivenArg_ReturnsExpectedResult) {
        int data = 5;
        auto task = MakeTask(TaskFuncWithArg, data);
        task.Process();
        EXPECT_EQ(task.GetResult(), 15);
    }
    TEST_F(TaskFixture, GetResult_GivenRefArg_ReturnsExpectedResult) {
        int data = 5;
        auto task = MakeTask(TaskFuncWithRefArg, std::ref(data), &mutex_);
        task.Process();
        task.GetResult();
        EXPECT_EQ(data, 15);
    }

    TEST_F(TaskFixture, GetResult_GivenConstRefArg_ReturnsExpectedResult) {
        int data = 5;
        auto task = MakeTask(TaskFuncWithConstRefArg, std::cref(data), &mutex_);
        task.Process();
        EXPECT_EQ(task.GetResult(), 15);
    }

    TEST_F(TaskFixture, GetResult_GivenVoidArg_ReturnsExpectedResult) {
        auto task = MakeTask(TaskFuncWithoutArg);
        task.Process();
        task.GetResult();
        EXPECT_EQ(task.GetTaskState(), TaskState::SUCCESS);
    }

    TEST_F(TaskFixture, Process_ReadAndWriteGeneralData_TasksDoNotConflictForData) {
        //cycle to test different threads distributions
        for (int thread_num = 0; thread_num < counter_of_threads_samples; ++thread_num) {
            int data = 10;
            auto task_read = MakeTask(TaskFuncWithConstRefArg, std::cref(data), &mutex_);
            auto task_write = MakeTask(TaskFuncWithRefArg, std::ref(data), &mutex_);

            task_write.Process();
            task_read.Process();
            task_write.GetResult();
            auto modified_result_in_read_mode = task_read.GetResult();

            EXPECT_EQ(data, 20);
            ASSERT_TRUE(modified_result_in_read_mode == 30 || modified_result_in_read_mode == 60);
        }
    }

    TEST_F(TaskFixture, ParallelExecutionProcess_GivenThreeTasks_ShouldCompleteFasterThanSequential) {
        for (int thread_num = 0; thread_num < counter_of_threads_samples; ++thread_num) {
            auto start = std::chrono::high_resolution_clock::now();

            auto task1 = MakeTask(TaskFuncWithoutArg);
            auto task2 = MakeTask(TaskFuncWithoutArg);
            auto task3 = MakeTask(TaskFuncWithoutArg);

            task1.Process();
            task2.Process();
            task3.Process();

            task1.GetResult();
            task2.GetResult();
            task3.GetResult();


            auto end = std::chrono::high_resolution_clock::now();
            auto duration_parallel = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            start = std::chrono::high_resolution_clock::now();

            TaskFuncWithoutArg();
            TaskFuncWithoutArg();
            TaskFuncWithoutArg();

            end = std::chrono::high_resolution_clock::now();
            auto duration_sequential = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            EXPECT_LT(duration_parallel, duration_sequential);
        }

    }

    TEST_F(TaskFixture, Process_GivenExceptionInTask_ShouldHandleExceptionAndSetErrorState) {
        auto task = MakeTask(TaskFuncException);
        task.Process();
        task.GetResult();
        EXPECT_EQ(task.GetTaskState(), TaskState::ERROR);
    }
}