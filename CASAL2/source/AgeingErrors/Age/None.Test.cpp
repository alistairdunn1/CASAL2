/**
 * @file None.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/03/27
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "None.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../Model/Factory.h"
#include "../../TestResources/MockClasses/Model.h"

namespace niwa {

using ::testing::_;
using ::testing::Return;

/**
 * Test the misclassification matrix generation for None ageing error
 */
TEST(AgeingErrors, None_MisMatrixGeneration) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(2));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::None none(mock_model);
  none.parameters().Add(PARAM_LABEL, PARAM_NONE, __FILE__, __LINE__);
  none.parameters().Add(PARAM_TYPE, PARAM_NONE, __FILE__, __LINE__);

  // The matrix is private, so we'll use the public method GetMisMatrix to access it
  none.Validate();
  none.Build();

  // Get the matrix
  auto matrix = none.mis_matrix();

  // For None, we expect a diagonal matrix of 1's and all other elements to be 0
  // Age spread should be (max_age - min_age) + 1 = (6 - 2) + 1 = 5
  ASSERT_EQ(5u, matrix.size());

  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(5u, matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      if (i == j) {
        EXPECT_DOUBLE_EQ(1.0, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be 1.0";
      } else {
        EXPECT_DOUBLE_EQ(0.0, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be 0.0";
      }
    }
  }
}

/**
 * Test the misclassification matrix with a different age range
 */
TEST(AgeingErrors, None_MisMatrixDifferentAgeRange) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(1));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(10));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(10));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::None none(mock_model);
  none.parameters().Add(PARAM_LABEL, PARAM_NONE, __FILE__, __LINE__);
  none.parameters().Add(PARAM_TYPE, PARAM_NONE, __FILE__, __LINE__);

  none.Validate();
  none.Build();

  auto matrix = none.mis_matrix();

  // Age spread should be (max_age - min_age) + 1 = (10 - 1) + 1 = 10
  ASSERT_EQ(10u, matrix.size());

  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(10u, matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      if (i == j) {
        EXPECT_DOUBLE_EQ(1.0, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be 1.0";
      } else {
        EXPECT_DOUBLE_EQ(0.0, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be 0.0";
      }
    }
  }
}

/**
 * Test the misclassification matrix without a plus group
 */
TEST(AgeingErrors, None_MisMatrixNoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(2));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));

  ageingerrors::None none(mock_model);
  none.parameters().Add(PARAM_LABEL, PARAM_NONE, __FILE__, __LINE__);
  none.parameters().Add(PARAM_TYPE, PARAM_NONE, __FILE__, __LINE__);

  none.Validate();
  none.Build();

  auto matrix = none.mis_matrix();

  // Test that the matrix is still a diagonal matrix of 1's
  // Age spread should still be (max_age - min_age) + 1 = (6 - 2) + 1 = 5
  ASSERT_EQ(5u, matrix.size());

  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(5u, matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      if (i == j) {
        EXPECT_DOUBLE_EQ(1.0, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be 1.0";
      } else {
        EXPECT_DOUBLE_EQ(0.0, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be 0.0";
      }
    }
  }
}

}  // namespace niwa
#endif  // TESTMODE