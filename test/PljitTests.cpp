//
// Created by Andreas Bauer on 14.08.22.
//

#include "pljit/pljit.hpp"
#include "./utils/assert_macros.hpp"
#include "./utils/CaptureCOut.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <thread>

//---------------------------------------------------------------------------
namespace pljit {
//---------------------------------------------------------------------------
TEST(Pljit, testExampleProgram) {
    Pljit pljit;

    auto func = pljit.registerFunction("PARAM width, height, depth;\n"
                                       "VAR volume;\n"
                                       "CONST density = 2400;\n"
                                       "BEGIN\n"
                                       "  volume := width * height * depth;\n"
                                       "  RETURN density * volume\n"
                                       "END.");

    auto result = func({100, 100, 100});
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 2400000000);

    result = func(100, 100, 100);
    ASSERT_TRUE(result);
    ASSERT_EQ(*result, 2400000000);
}

TEST(Pljit, testMultiThreadedExecution) {
    Pljit pljit;
    auto func = pljit.registerFunction("PARAM width, height, depth;\n"
                                       "VAR volume;\n"
                                       "CONST density = 2400;\n"
                                       "BEGIN\n"
                                       "  volume := width * height * depth;\n"
                                       "  RETURN density * volume\n"
                                       "END.");

    for (unsigned round = 0; round < 2; ++round) {
        unsigned count = 8;
        std::vector<std::optional<long long>> results{};
        results.resize(count);
        std::vector<std::thread> threads;
        threads.reserve(count);

        for (unsigned i = 0; i < count; ++i) {
            threads.emplace_back([func, i, &results]() {
                auto result = func(10, 10, 10);
                results[i] = result;
            });
        }

        for (auto& thread: threads) {
            thread.join();
        }

        for (auto& result: results) {
            ASSERT_TRUE(result);
            ASSERT_EQ(*result, 2400000);
        }
    }
}

TEST(Pljit, testMultiThreadedCompilerErrors) {
    // parser error! (missing CONST init)
    {
        Pljit pljit;
        auto func = pljit.registerFunction("CONST a;\n"
                                           "BEGIN\n"
                                           "  RETURN a"
                                           "END.");

        unsigned count = 8;
        std::vector<std::optional<long long>> results{};
        results.resize(count);
        std::vector<std::thread> threads;
        threads.reserve(count);

        for (unsigned i = 0; i < count; ++i) {
            threads.emplace_back([func, i, &results]() {
                auto result = func(10, 10, 10);
                results[i] = result;
            });
        }

        for (auto& thread: threads) {
            thread.join();
        }

        for (auto& result: results) {
            ASSERT_FALSE(result);
        }
        ASSERT_SRC_ERROR_CONTENTS(*func.compilation_error(), code::CodePosition(1, 8), "Expected `=` operator!", ";");
    }
    // syntax error! (missing return)
    {
        Pljit pljit;
        auto func = pljit.registerFunction("VAR a;\n"
                                           "BEGIN\n"
                                           "  a := 1\n"
                                           "END.");

        unsigned count = 8;
        std::vector<std::optional<long long>> results{};
        results.resize(count);
        std::vector<std::thread> threads;
        threads.reserve(count);

        for (unsigned i = 0; i < count; ++i) {
            threads.emplace_back([func, i, &results]() {
                auto result = func(10, 10, 10);
                results[i] = result;
            });
        }

        for (auto& thread: threads) {
            thread.join();
        }

        for (auto& result: results) {
            ASSERT_FALSE(result);
        }
        ASSERT_SRC_ERROR_CONTENTS(*func.compilation_error(), code::CodePosition(4, 1), "Reached end of function without a RETURN statement!", "END");
    }
}

TEST(Pljit, testMultiThreadedRegistration) {
    Pljit pljit;

    unsigned thread_count = 8;
    std::vector<std::thread> threads;
    threads.reserve(thread_count);

    for (unsigned i = 0; i < thread_count; ++i) {
        threads.emplace_back([&pljit]() {
            pljit.registerFunction("BEGIN RETURN 0 END.");
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    std::size_t count = 0;
    auto head = pljit.list_head;
    while (head != nullptr) {
        ++count;
        head = head->next;
    }

    ASSERT_EQ(count, 8);
}

TEST(Pljit, testErrorPrinting) {
    Pljit pljit;
    auto func = pljit.registerFunction("BEGIN RETURN 1 / 0 END.");

    CaptureCOut capture;
    auto result = func();
    capture.stopCapture();

    ASSERT_FALSE(result);
    ASSERT_EQ(capture.str(), "Division by zero!\n");
}
//---------------------------------------------------------------------------
} // namespace pljit
//---------------------------------------------------------------------------
