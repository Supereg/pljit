//
// Created by Andreas Bauer on 19.08.22.
//

#ifndef PLJIT_ASSERT_MACROS_HPP
#define PLJIT_ASSERT_MACROS_HPP

#include <iostream>

#define ASSERT_TOKEN(result, type, exp_content) \
    ASSERT_TRUE((result).isSuccess()); \
    EXPECT_EQ((result).value().getType(), (type)); \
    ASSERT_EQ(*(result).value().reference(), (exp_content))

#define ASSERT_NEXT_TOKEN(lexer, result, type, exp_content) \
    (result) = (lexer).consume_next(); \
    ASSERT_TOKEN(result, type, exp_content)

#define ASSERT_SRC_ERROR_CONTENTS(error, exp_position, exp_message, exp_content) \
    EXPECT_EQ((error).position(), (exp_position)); \
    EXPECT_EQ((error).message(), (exp_message)); \
    EXPECT_EQ(*(error).reference(), (exp_content))

#define ASSERT_SRC_ERROR(result, exp_position, exp_message, exp_content) \
    ASSERT_TRUE((result).isFailure()); \
    ASSERT_SRC_ERROR_CONTENTS((result).error(), exp_position, exp_message, exp_content)

#define ASSERT_PARSE_TREE(tree, expected_tree) \
    do { \
        DOTVisitor visitor; \
        CaptureCOut capture; \
        visitor.print(tree); \
        ASSERT_EQ(capture.str(), expected_tree); \
    } while(0)

#endif //PLJIT_ASSERT_MACROS_HPP
