/**
 * @file OffByOne.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/01
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "OffByOne.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../Model/Factory.h"
#include "../../TestResources/MockClasses/Model.h"

namespace niwa {

using ::testing::_;
using ::testing::Return;

/**
 * Test the misclassification matrix generation for OffByOne ageing error
 */
TEST(AgeingErrors, OffByOne_MisMatrixGeneration) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.1", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.2", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);

  offbyone.Validate();
  offbyone.Build();

  // Get the matrix
  auto matrix = offbyone.mis_matrix();

  // Age spread should be (max_age - min_age) + 1 = (7 - 3) + 1 = 5
  ASSERT_EQ(5u, matrix.size());

  // Test for expected values for first row (age 3)
  // Based on p1=0.1, p2=0.2
  EXPECT_DOUBLE_EQ(0.8, matrix[0][0]);  // 1.0 - p2
  EXPECT_DOUBLE_EQ(0.2, matrix[0][1]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[0][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][4]);

  // Test middle row (age 5)
  EXPECT_DOUBLE_EQ(0.1, matrix[2][1]);  // p1
  EXPECT_DOUBLE_EQ(0.7, matrix[2][2]);  // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.2, matrix[2][3]);  // p2

  // Test last row (age 7) with plus group
  EXPECT_DOUBLE_EQ(0.0, matrix[4][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[4][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[4][2]);
  EXPECT_DOUBLE_EQ(0.1, matrix[4][3]);  // p1
  EXPECT_DOUBLE_EQ(0.9, matrix[4][4]);  // 1.0 - p1 (with plus group)

  // Check that each row sums to 1.0
  for (unsigned i = 0; i < matrix.size(); ++i) {
    double row_sum = 0.0;
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      row_sum += matrix[i][j];
    }
    EXPECT_NEAR(1.0, row_sum, 0.001) << "Row " << i << " does not sum to 1.0";
  }
}

/**
 * Test the misclassification matrix generation with different p1 and p2 values
 */
TEST(AgeingErrors, OffByOne_DifferentPs) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_different_ps", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.3", __FILE__, __LINE__);  // Higher p1
  offbyone.parameters().Add(PARAM_P2, "0.1", __FILE__, __LINE__);  // Lower p2
  offbyone.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // Test with different p1 and p2 values
  // First row (age 3)
  EXPECT_DOUBLE_EQ(0.9, matrix[0][0]);  // 1.0 - p2
  EXPECT_DOUBLE_EQ(0.1, matrix[0][1]);  // p2

  // Middle row (age 5)
  EXPECT_DOUBLE_EQ(0.3, matrix[2][1]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[2][2]);  // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.1, matrix[2][3]);  // p2

  // Last row (age 7) with plus group
  EXPECT_DOUBLE_EQ(0.3, matrix[4][3]);  // p1
  EXPECT_DOUBLE_EQ(0.7, matrix[4][4]);  // 1.0 - p1 (with plus group)
}

/**
 * Test the misclassification matrix with k parameter
 */
TEST(AgeingErrors, OffByOne_WithK) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_k", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.1", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.2", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "5", __FILE__, __LINE__);  // Ages < 5 have no error

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // Ages < 5 should have no error (diagonal = 1.0, all other elements = 0.0)
  // Age 3 (first row)
  EXPECT_DOUBLE_EQ(1.0, matrix[0][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][4]);

  // Age 4 (second row)
  EXPECT_DOUBLE_EQ(0.0, matrix[1][0]);
  EXPECT_DOUBLE_EQ(1.0, matrix[1][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][4]);

  // Age 5 (third row, k=5) should have normal error pattern
  EXPECT_DOUBLE_EQ(0.0, matrix[2][0]);
  EXPECT_DOUBLE_EQ(0.1, matrix[2][1]);  // p1
  EXPECT_DOUBLE_EQ(0.7, matrix[2][2]);  // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.2, matrix[2][3]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[2][4]);
}

/**
 * Test the misclassification matrix without plus group
 */
TEST(AgeingErrors, OffByOne_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_no_plus", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.1", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.2", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // The last row should be different without plus group
  // Last row (age 7) - with no plus group
  EXPECT_DOUBLE_EQ(0.0, matrix[4][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[4][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[4][2]);
  EXPECT_DOUBLE_EQ(0.1, matrix[4][3]);  // p1
  EXPECT_DOUBLE_EQ(0.7, matrix[4][4]);  // 1.0 - (p1 + p2) (without plus group)

  // Check that the last row still sums to less than 1.0 (or equal) without plus group
  double last_row_sum = 0.0;
  for (unsigned j = 0; j < matrix[4].size(); ++j) {
    last_row_sum += matrix[4][j];
  }
  EXPECT_NEAR(0.8, last_row_sum, 0.001);  // 0.1 + 0.7 = 0.8
}

/**
 * Test the specific expected matrix values for OffByOne ageing error with min_age=3, max_age=8
 */
TEST(AgeingErrors, OffByOne_ExpectedMatrix_PlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_matrix", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.15", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.25", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // Expected 6x6 matrix
  ASSERT_EQ(6u, matrix.size());
  for (unsigned i = 0; i < 6; ++i) {
    ASSERT_EQ(6u, matrix[i].size());
  }

  // Check specific values against the expected matrix
  // Row 1 (age 3)
  EXPECT_DOUBLE_EQ(0.75, matrix[0][0]);  // 1.0 - p2
  EXPECT_DOUBLE_EQ(0.25, matrix[0][1]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[0][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][4]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][5]);

  // Row 2 (age 4)
  EXPECT_DOUBLE_EQ(0.15, matrix[1][0]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[1][1]);   // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.25, matrix[1][2]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[1][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][4]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][5]);

  // Row 3 (age 5)
  EXPECT_DOUBLE_EQ(0.0, matrix[2][0]);
  EXPECT_DOUBLE_EQ(0.15, matrix[2][1]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[2][2]);   // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.25, matrix[2][3]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[2][4]);
  EXPECT_DOUBLE_EQ(0.0, matrix[2][5]);

  // Row 4 (age 6)
  EXPECT_DOUBLE_EQ(0.0, matrix[3][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[3][1]);
  EXPECT_DOUBLE_EQ(0.15, matrix[3][2]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[3][3]);   // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.25, matrix[3][4]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[3][5]);

  // Row 5 (age 7)
  EXPECT_DOUBLE_EQ(0.0, matrix[4][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[4][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[4][2]);
  EXPECT_DOUBLE_EQ(0.15, matrix[4][3]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[4][4]);   // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.25, matrix[4][5]);  // p2

  // Row 6 (age 8, plus group)
  EXPECT_DOUBLE_EQ(0.0, matrix[5][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][3]);
  EXPECT_DOUBLE_EQ(0.15, matrix[5][4]);  // p1
  EXPECT_DOUBLE_EQ(0.85, matrix[5][5]);  // 1.0 - p1 (with plus group)

  // Verify matrix application on a population vector
  vector<double> population = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += population[i] * matrix[i][j];
    }
  }

  // Check against expected execute results
  EXPECT_NEAR(10.5, result[0], 0.00001);   // Age 3 = 10.0 (Age 3: 10 * 1.0)
  EXPECT_NEAR(19.0, result[1], 0.00001);   // Age 4 = 20.0 (Age 4: 20 * 1.0) - 1.0 (Age 5: 30 * 0.05)
  EXPECT_NEAR(26.0, result[2], 0.00001);   // Age 5 = 18.0 (Age 5: 30 * 0.6) + 8.0 (Age 6: 20 * 0.4)
  EXPECT_NEAR(21.75, result[3], 0.00001);  // Age 6 = 7.5 (Age 5: 30 * 0.25) + 12.0 (Age 6: 20 * 0.6) + 2.25 (Age 7: 15 * 0.15)
  EXPECT_NEAR(14.75, result[4], 0.00001);  // Age 7 = 5.0 (Age 6: 20 * 0.25) + 9.0 (Age 7: 15 * 0.6) + 0.75 (Age 8: 5 * 0.15)
  EXPECT_NEAR(8.0, result[5], 0.00001);    // Age 8 = 4.25 (Age 8: 5 * 0.85) + 3.75 (Age 7: 15 * 0.25)
}

/**
 * Test the specific expected matrix values for OffByOne ageing error with k=5
 */
TEST(AgeingErrors, OffByOne_ExpectedMatrix_WithK) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_matrix_k", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.15", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.25", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "5", __FILE__, __LINE__);  // Ages < 5 have no error

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // Row 1 (age 3) should have no error (identity matrix row)
  EXPECT_DOUBLE_EQ(1.0, matrix[0][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][4]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][5]);

  // Row 2 (age 4) should have no error (identity matrix row)
  EXPECT_DOUBLE_EQ(0.0, matrix[1][0]);
  EXPECT_DOUBLE_EQ(1.0, matrix[1][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][3]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][4]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][5]);

  // Row 3 (age 5) should have error pattern
  EXPECT_DOUBLE_EQ(0.0, matrix[2][0]);
  EXPECT_DOUBLE_EQ(0.15, matrix[2][1]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[2][2]);   // 1.0 - (p1 + p2)
  EXPECT_DOUBLE_EQ(0.25, matrix[2][3]);  // p2
  EXPECT_DOUBLE_EQ(0.0, matrix[2][4]);
  EXPECT_DOUBLE_EQ(0.0, matrix[2][5]);

  // Verify matrix application on a population vector with k=5
  // Ages:                     3     4     5     6     7     8
  vector<double> population = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += population[i] * matrix[i][j];
    }
  }
  /**
   * Matrix
   * Ages 3   4    5    6    7    8
   * 3    1   0    0    0    0    0
   * 4    0   1    0    0    0    0
   * 5    0   0.15 0.6  0.25 0    0
   * 6    0   0    0.15 0.6  0.25 0
   * 7    0   0    0    0.15 0.6  0.25
   * 8    0   0    0    0    0.15 0.85
   */

  // Check against expected execute results with k=5
  EXPECT_NEAR(10.0, result[0], 0.00001);   // Age 3 = Only from age 3 (identity)
  EXPECT_NEAR(24.5, result[1], 0.00001);   // Age 4 = 20.0 from age 4 (identity) + 4.5 from age 5
  EXPECT_NEAR(21.0, result[2], 0.00001);   // Age 5 = 18.0 (Age 5: 30 * 0.6) + 3 (20 * .15)
  EXPECT_NEAR(21.75, result[3], 0.00001);  // Age 6 = 7.5 (Age 5: 30 * 0.25) + 12 (Age 6: 20 * .6) + 2.25 (Age 7: 15 * 0.15)
  EXPECT_NEAR(14.75, result[4], 0.00001);  // Age 7 = 5 (Age 6: 20 * 0.25) + 9 (Age 7: 15 * 0.6) + 0.75 (Age 8: 5 * 0.15)
  EXPECT_NEAR(8.0, result[5], 0.00001);    // Age 8 = X (Age 7: 15 * .25) + 4.25 (Age 8: 5 * 0.85)
}

/**
 * Test the specific expected matrix values for OffByOne ageing error without plus group
 */
TEST(AgeingErrors, OffByOne_ExpectedMatrix_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_matrix_no_plus", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.15", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.25", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // Most rows should be the same as with plus group
  // Row 1-5 (ages 3-7) should be the same

  // Row 6 (age 8) is different with no plus group
  EXPECT_DOUBLE_EQ(0.0, matrix[5][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][3]);
  EXPECT_DOUBLE_EQ(0.15, matrix[5][4]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[5][5]);   // 1.0 - (p1 + p2) (without plus group)

  // Verify matrix application on a population vector without plus group
  vector<double> population = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += population[i] * matrix[i][j];
    }
  }

  // Check against expected execute results with no plus group
  EXPECT_NEAR(10.5, result[0], 0.00001);   // 10.0 (Age 3: 10 * 1.0) + 0.5 (Age 4: 20 * 0.025)
  EXPECT_NEAR(19.0, result[1], 0.00001);   // 18.0 (Age 4: 20 * 0.9) + 1.0 (Age 5: 30 * 0.0333)
  EXPECT_NEAR(26.0, result[2], 0.00001);   // 24.0 (Age 5: 30 * 0.8) + 2.0 (Age 6: 20 * 0.1)
  EXPECT_NEAR(21.75, result[3], 0.00001);  // 18.0 (Age 6: 20 * 0.9) + 3.75 (Age 7: 15 * 0.25)
  EXPECT_NEAR(14.75, result[4], 0.00001);  // 12.0 (Age 7: 15 * 0.8) + 2.75 (Age 8: 5 * 0.55)
  EXPECT_NEAR(6.75, result[5], 0.00001);   // 3.0 (Age 8: 5 * 0.6) + 3.75 (Age 7: 15 * 0.25)
}

/**
 * Test the specific expected matrix values for OffByOne ageing error with k=5 and no plus group
 */
TEST(AgeingErrors, OffByOne_ExpectedMatrix_WithK_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::OffByOne offbyone(mock_model);
  offbyone.parameters().Add(PARAM_LABEL, "test_offbyone_matrix_k_no_plus", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_TYPE, "off_by_one", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P1, "0.15", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_P2, "0.25", __FILE__, __LINE__);
  offbyone.parameters().Add(PARAM_K, "5", __FILE__, __LINE__);  // Ages < 5 have no error

  offbyone.Validate();
  offbyone.Build();

  auto matrix = offbyone.mis_matrix();

  // Row 1 (age 3) and Row 2 (age 4) should have no error (identity matrix rows)
  EXPECT_DOUBLE_EQ(1.0, matrix[0][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[0][1]);
  EXPECT_DOUBLE_EQ(1.0, matrix[1][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[1][2]);

  // Row 6 (age 8) is different with no plus group
  EXPECT_DOUBLE_EQ(0.0, matrix[5][0]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][1]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][2]);
  EXPECT_DOUBLE_EQ(0.0, matrix[5][3]);
  EXPECT_DOUBLE_EQ(0.15, matrix[5][4]);  // p1
  EXPECT_DOUBLE_EQ(0.6, matrix[5][5]);   // 1.0 - (p1 + p2) (without plus group)

  // Verify matrix application on a population vector with k=5 and no plus group
  vector<double> population = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += population[i] * matrix[i][j];
    }
  }

  // Check against expected execute results with k=5 and no plus group
  EXPECT_NEAR(10.0, result[0], 0.00001);   // Age 3 = Only from age 3 (identity)
  EXPECT_NEAR(24.5, result[1], 0.00001);   // Age 4 = 20.0 from age 4 (identity) + 4.5 from age 5 (30 * 0.15)
  EXPECT_NEAR(21.0, result[2], 0.00001);   // Age 5 = 18.0 from age 5 (30 * 0.6) + 3.0 from age 6 (20 * 0.15)
  EXPECT_NEAR(21.75, result[3], 0.00001);  // Age 6 = 7.5 from age 5 (30 * 0.25) + 12.0 from age 6 (20 * 0.6) + 2.25 from age 7 (15 * 0.15)
  EXPECT_NEAR(14.75, result[4], 0.00001);  // Age 7 = 5.0 from age 6 (20 * 0.25) + 9.0 from age 7 (15 * 0.6) + 0.75 from age 8 (5 * 0.15)
  EXPECT_NEAR(6.75, result[5], 0.00001);   // Age 8 = 3.0 from age 7 (15 * 0.25) + 3.75 from age 8 (5 * 0.6)
}

}  // namespace niwa
#endif  // TESTMODE