/**
 * @file String.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 12/09/2014
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 *
 * @section DESCRIPTION
 *
 * << Add Description >>
 */
#ifdef TESTMODE

// headers
#include "String.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// namespaces
namespace niwa {
namespace utilities {

using ::testing::Return;

/**
 * Test the results of our selectivity are correct
 */
TEST(Utilities, String) {
  string test = "good_label";
  EXPECT_EQ("", String::find_invalid_characters(test));

  test = "bad!!label";
  EXPECT_EQ("!!", String::find_invalid_characters(test));

  test = "good.-_";
  EXPECT_EQ("", String::find_invalid_characters(test));

  test = "bad%la^bel!";
  EXPECT_EQ("%^!", String::find_invalid_characters(test));
}

} /* namespace utilities */
} /* namespace niwa */

#endif
