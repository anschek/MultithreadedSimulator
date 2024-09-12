#include "pch.h"
#include "gtest/gtest.h" //Google Test
#include "../MultithreadedSimulator/Task.h"

#include<numeric>
#include<typeinfo>
#include <type_traits>
TEST(TaskTest, Process_GivenSimpleArgs_ReturnsExpectedResult) {
    // Arrange
    auto v = std::vector<int>{ 5, 5, 6 };
    auto task = MakeTask([](std::vector<int> v) {
        return std::accumulate(v.begin(), v.end(), 0);
        },ref(v)
    );
    // Act
    task.Process();
    auto result = task.GetResult();
    // Assert
    ASSERT_EQ(result, 16);
}

double returnRandom() {
    std::srand(std::time(nullptr));
    return std::rand() / 2.718 * std::rand();
}

TEST(TaskTest, Process_GivenVoidArgs_ReturnsExpectedResult) {
    // Arrange
    auto task = MakeTask(returnRandom);
    // Act
    task.Process();
    auto result = task.GetResult();
    // Assert
    static_assert(std::is_same<decltype(result), double>::value, "type of the result variable must be double");
}