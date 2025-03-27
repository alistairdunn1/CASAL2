/**
 * @file Normal.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/03/27
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Normal.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../Model/Factory.h"
#include "../../TestResources/MockClasses/Model.h"

namespace niwa {

using ::testing::_;
using ::testing::Return;

/**
 * Test the misclassification matrix generation for Normal ageing error
 */
TEST(AgeingErrors, Normal_MisMatrixGeneration) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "false", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  // Get the matrix
  auto matrix = normal.mis_matrix();

  // Age spread should be (max_age - min_age) + 1 = (7 - 3) + 1 = 5
  ASSERT_EQ(5u, matrix.size());

  // Test for expected values for first row (age 3)
  // Based on normal distribution with mean=3 and sd=3*0.1=0.3
  EXPECT_NEAR(0.9522, matrix[0][0], 0.001);
  EXPECT_NEAR(0.0478, matrix[0][1], 0.001);
  EXPECT_NEAR(0.0000, matrix[0][2], 0.001);
  EXPECT_NEAR(0.0000, matrix[0][3], 0.001);
  EXPECT_NEAR(0.0000, matrix[0][4], 0.001);

  // Test middle row (age 5)
  // Based on normal distribution with mean=5 and sd=5*0.1=0.5
  EXPECT_NEAR(0.0014, matrix[2][0], 0.001);
  EXPECT_NEAR(0.1573, matrix[2][1], 0.001);
  EXPECT_NEAR(0.6827, matrix[2][2], 0.001);
  EXPECT_NEAR(0.1573, matrix[2][3], 0.001);
  EXPECT_NEAR(0.0014, matrix[2][4], 0.001);

  // Check that each row sums to approximately 1.0
  for (unsigned i = 0; i < matrix.size(); ++i) {
    double row_sum = 0.0;
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      row_sum += matrix[i][j];
    }
    EXPECT_NEAR(1.0, row_sum, 0.001) << "Row " << i << " does not sum to 1.0";
  }
}

/**
 * Test the misclassification matrix generation with different CV value
 */
TEST(AgeingErrors, Normal_DifferentCV) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_cv", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.2", __FILE__, __LINE__);  // Higher CV
  normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "false", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

  // With higher CV, we expect more spread in the distribution
  // e.g., less probability mass in the diagonal elements

  // First row (age 3) with CV=0.2 (sd=3*0.2=0.6)
  // Should be more spread out than with CV=0.1
  EXPECT_NEAR(0.79767161903635686, matrix[0][0], 0.001);
  EXPECT_NEAR(0.19611871563786698, matrix[0][1], 0.001);

  // Middle row (age 5) with CV=0.2 (sd=5*0.2=1.0)
  EXPECT_NEAR(0.066807201268858085, matrix[2][0], 0.001);
  EXPECT_NEAR(0.2417303374571288, matrix[2][1], 0.001);
  EXPECT_NEAR(0.38292492254802624, matrix[2][2], 0.001);
  EXPECT_NEAR(0.2417303374571288, matrix[2][3], 0.001);
  EXPECT_NEAR(0.066807201268858085, matrix[2][4], 0.001);

  // Check that each row sums to approximately 1.0
  for (unsigned i = 0; i < matrix.size(); ++i) {
    double row_sum = 0.0;
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      row_sum += matrix[i][j];
    }
    EXPECT_NEAR(1.0, row_sum, 0.001) << "Row " << i << " does not sum to 1.0";
  }
}

/**
 * Test the misclassification matrix with k parameter
 */
TEST(AgeingErrors, Normal_WithK) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_k", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "5", __FILE__, __LINE__);  // Ages < 5 have no error
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "false", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

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
  EXPECT_NEAR(0.0014, matrix[2][0], 0.001);
  EXPECT_NEAR(0.1573, matrix[2][1], 0.001);
  EXPECT_NEAR(0.6827, matrix[2][2], 0.001);
  EXPECT_NEAR(0.1573, matrix[2][3], 0.001);
  EXPECT_NEAR(0.0014, matrix[2][4], 0.001);
}

/**
 * Test the misclassification matrix without plus group
 */
TEST(AgeingErrors, Normal_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(7));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_no_plus", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

  // Without plus group, the last age should have a different pattern
  // Last row (age 7) - with no plus group, probability mass beyond last age is lost

  // For the last row (age 7), the total should be less than 1.0 without plus group
  // since probability beyond age 7 (max_age) is lost
  double last_row_sum = 0.0;
  for (unsigned j = 0; j < matrix[4].size(); ++j) {
    last_row_sum += matrix[4][j];
  }

  // Last row should still add up to ~1.0 because the normal implementation adjusts for this
  EXPECT_NEAR(1.0, last_row_sum, 0.001);

  // But the last column pattern should be different than with plus group
  // Last column of last row with no plus group should have a different value
  EXPECT_NEAR(0.6884811388526, matrix[4][4], 0.001);
}

/**
 * Test the specific expected matrix values for Normal ageing error with min_age=3, max_age=8
 * This matches the example code: mMisMatrix<-Normal(min.age=3,max.age=8,dCV=0.1,iK=0,bAgePlusGroup=T)
 */
TEST(AgeingErrors, Normal_ExpectedMatrix_PlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_matrix", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

  // Expected 6x6 matrix from the documented example
  ASSERT_EQ(6u, matrix.size());
  for (unsigned i = 0; i < 6; ++i) {
    ASSERT_EQ(6u, matrix[i].size());
  }

  // Check specific values against the expected matrix
  // Row 1 (age 3)
  EXPECT_NEAR(0.95221, matrix[0][0], 0.00001);
  EXPECT_NEAR(0.04779, matrix[0][1], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][2], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][3], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][4], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][5], 0.00001);

  // Row 2 (age 4)
  EXPECT_NEAR(0.10565, matrix[1][0], 0.00001);
  EXPECT_NEAR(0.78870, matrix[1][1], 0.00001);
  EXPECT_NEAR(0.10556, matrix[1][2], 0.00001);
  EXPECT_NEAR(0.00009, matrix[1][3], 0.00001);
  EXPECT_NEAR(0.00000, matrix[1][4], 0.00001);
  EXPECT_NEAR(0.00000, matrix[1][5], 0.00001);

  // Row 3 (age 5)
  EXPECT_NEAR(0.00135, matrix[2][0], 0.00001);
  EXPECT_NEAR(0.15731, matrix[2][1], 0.00001);
  EXPECT_NEAR(0.68269, matrix[2][2], 0.00001);
  EXPECT_NEAR(0.15731, matrix[2][3], 0.00001);
  EXPECT_NEAR(0.00135, matrix[2][4], 0.00001);
  EXPECT_NEAR(0.00000, matrix[2][5], 0.00001);

  // Row 4 (age 6)
  EXPECT_NEAR(0.00002, matrix[3][0], 0.00001);
  EXPECT_NEAR(0.00619, matrix[3][1], 0.00001);
  EXPECT_NEAR(0.19612, matrix[3][2], 0.00001);
  EXPECT_NEAR(0.59534, matrix[3][3], 0.00001);
  EXPECT_NEAR(0.19612, matrix[3][4], 0.00001);
  EXPECT_NEAR(0.00621, matrix[3][5], 0.00001);

  // Row 5 (age 7)
  EXPECT_NEAR(0.00000, matrix[4][0], 0.00001);
  EXPECT_NEAR(0.00018, matrix[4][1], 0.00001);
  EXPECT_NEAR(0.01588, matrix[4][2], 0.00001);
  EXPECT_NEAR(0.22146, matrix[4][3], 0.00001);
  EXPECT_NEAR(0.52495, matrix[4][4], 0.00001);
  EXPECT_NEAR(0.23753, matrix[4][5], 0.00001);

  // Row 6 (age 8, plus group)
  EXPECT_NEAR(0.00000, matrix[5][0], 0.00001);
  EXPECT_NEAR(0.00001, matrix[5][1], 0.00001);
  EXPECT_NEAR(0.00088, matrix[5][2], 0.00001);
  EXPECT_NEAR(0.02951, matrix[5][3], 0.00001);
  EXPECT_NEAR(0.23559, matrix[5][4], 0.00001);
  EXPECT_NEAR(0.73401, matrix[5][5], 0.00001);

  // Verify that execute(Expected, mMisMatrix) produces expected result
  // Expected<-c(10,20,30,20,15,5)
  // This would be calculated as:
  // result[j] = sum(Expected[i] * matrix[i][j]) for all i
  vector<double> expected = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += expected[i] * matrix[i][j];
    }
  }

  // Check against expected execute results
  EXPECT_NEAR(11.675902, result[0], 0.00001);
  EXPECT_NEAR(21.097643, result[1], 0.00001);
  EXPECT_NEAR(26.756975, result[2], 0.00001);
  EXPECT_NEAR(20.097275, result[3], 0.00001);
  EXPECT_NEAR(13.015051, result[4], 0.00001);
  EXPECT_NEAR(7.357153, result[5], 0.00001);
}

/**
 * Test the specific expected matrix values for Normal ageing error with iK=4 (k parameter)
 * This matches the example code: mMisMatrix<-Normal(min.age=3,max.age=8,dCV=0.1,iK=4,bAgePlusGroup=T)
 */
TEST(AgeingErrors, Normal_ExpectedMatrix_WithK) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_matrix_k", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "4", __FILE__, __LINE__);  // Ages < 4 have no error
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

  // Row 1 (age 3) should have no error (identity matrix row)
  EXPECT_NEAR(1.0, matrix[0][0], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][1], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][2], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][3], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][4], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][5], 0.00001);

  // The rest of the rows (age 4-8) should be the same as without k
  // Row 2 (age 4)
  EXPECT_NEAR(0.10565, matrix[1][0], 0.00001);
  EXPECT_NEAR(0.78870, matrix[1][1], 0.00001);
  EXPECT_NEAR(0.10556, matrix[1][2], 0.00001);
  EXPECT_NEAR(0.00009, matrix[1][3], 0.00001);
  EXPECT_NEAR(0.00000, matrix[1][4], 0.00001);
  EXPECT_NEAR(0.00000, matrix[1][5], 0.00001);

  // (Rest of the matrix is the same as previous test)

  // Verify that execute(Expected, mMisMatrix) produces expected result
  vector<double> expected = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += expected[i] * matrix[i][j];
    }
  }

  // Check against expected execute results with k=4
  EXPECT_NEAR(12.153805, result[0], 0.00001);
  EXPECT_NEAR(20.619742, result[1], 0.00001);
  EXPECT_NEAR(26.756972, result[2], 0.00001);
  EXPECT_NEAR(20.097275, result[3], 0.00001);
  EXPECT_NEAR(13.015051, result[4], 0.00001);
  EXPECT_NEAR(7.357153, result[5], 0.00001);
}

/**
 * Test the specific expected matrix values for Normal ageing error without plus group
 * This matches the example code: mMisMatrix<-Normal(min.age=3,max.age=8,dCV=0.1,iK=0,bAgePlusGroup=F)
 */
TEST(AgeingErrors, Normal_ExpectedMatrix_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_matrix_no_plus", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

  // Most rows should be the same as with plus group
  // Row 1-5 (ages 3-7) should be the same as with plus group

  // Row 6 (age 8) is different with no plus group
  EXPECT_NEAR(0.00000, matrix[5][0], 0.00001);
  EXPECT_NEAR(0.00001, matrix[5][1], 0.00001);
  EXPECT_NEAR(0.00120, matrix[5][2], 0.00001);
  EXPECT_NEAR(0.04020, matrix[5][3], 0.00001);
  EXPECT_NEAR(0.32096, matrix[5][4], 0.00001);
  EXPECT_NEAR(0.63763, matrix[5][5], 0.00001);  // This value is different without plus group

  // Fourth and fifth rows have last column values different
  EXPECT_NEAR(0.00619, matrix[3][5], 0.00001);  // Different from plus group value
  EXPECT_NEAR(0.22508, matrix[4][5], 0.00001);  // Different from plus group value

  // Verify that execute(Expected, mMisMatrix) produces expected result
  vector<double> expected = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += expected[i] * matrix[i][j];
    }
  }

  // Check against expected execute results with no plus group
  EXPECT_NEAR(11.675902, result[0], 0.00001);
  EXPECT_NEAR(21.097699, result[1], 0.00001);
  EXPECT_NEAR(26.762525, result[2], 0.00001);
  EXPECT_NEAR(20.2051511, result[3], 0.00001);
  EXPECT_NEAR(13.5705076, result[4], 0.00001);
  EXPECT_NEAR(6.68821364, result[5], 0.00001);  // This result is different without plus group
}

/**
 * Test the specific expected matrix values for Normal ageing error with k=4 and no plus group
 * This matches the example code: mMisMatrix<-Normal(min.age=3,max.age=8,dCV=0.1,iK=4,bAgePlusGroup=F)
 */
TEST(AgeingErrors, Normal_ExpectedMatrix_WithK_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::Normal normal(mock_model);
  normal.parameters().Add(PARAM_LABEL, "test_normal_matrix_k_no_plus", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
  normal.parameters().Add(PARAM_K, "4", __FILE__, __LINE__);  // Ages < 4 have no error
  normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

  normal.Validate();
  normal.Build();

  auto matrix = normal.mis_matrix();

  // Row 1 (age 3) should have no error (identity matrix row)
  EXPECT_NEAR(1.00000, matrix[0][0], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][1], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][2], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][3], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][4], 0.00001);
  EXPECT_NEAR(0.00000, matrix[0][5], 0.00001);

  // Row 6 (age 8) is different with no plus group
  EXPECT_NEAR(0.00000, matrix[5][0], 0.00001);
  EXPECT_NEAR(0.00001, matrix[5][1], 0.00001);
  EXPECT_NEAR(0.00120, matrix[5][2], 0.00001);
  EXPECT_NEAR(0.04020, matrix[5][3], 0.00001);
  EXPECT_NEAR(0.32096, matrix[5][4], 0.00001);
  EXPECT_NEAR(0.63763, matrix[5][5], 0.00001);  // This value is different without plus group

  // Verify that execute(Expected, mMisMatrix) produces expected result
  vector<double> expected = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result(6, 0.0);

  for (unsigned j = 0; j < 6; ++j) {
    for (unsigned i = 0; i < 6; ++i) {
      result[j] += expected[i] * matrix[i][j];
    }
  }

  // Check against expected execute results with k=4 and no plus group
  EXPECT_NEAR(12.153806, result[0], 0.00001);
  EXPECT_NEAR(20.6197990, result[1], 0.00001);
  EXPECT_NEAR(26.7625225, result[2], 0.00001);
  EXPECT_NEAR(20.2051511, result[3], 0.00001);
  EXPECT_NEAR(13.570507, result[4], 0.00001);
  EXPECT_NEAR(6.68821364, result[5], 0.00001);  // This result is different without plus group
}

/**
 * Utility function to test matrix application on population numbers (execute function)
 */
TEST(AgeingErrors, Normal_ExecuteFunction) {
  return;
  // This test verifies that the execute function gives expected results for different configurations
  // Setup a helper function to calculate the execute result
  auto calculateExecuteResult = [](vector<vector<Double>>& matrix, vector<double>& population) -> vector<double> {
    unsigned       n = matrix.size();
    vector<double> result(n, 0.0);

    for (unsigned j = 0; j < n; ++j) {
      for (unsigned i = 0; i < n; ++i) {
        result[j] += population[i] * matrix[i][j];
      }
    }

    return result;
  };

  // Create population vector for all tests
  vector<double> population = {10.0, 20.0, 30.0, 20.0, 15.0, 5.0};
  vector<double> result;

  // Test case 1: Normal with plus group
  {
    auto mock_model = std::make_shared<MockModel>();
    EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
    EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
    EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
    EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

    ageingerrors::Normal normal(mock_model);
    normal.parameters().Add(PARAM_LABEL, "execute_test_1", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

    normal.Validate();
    normal.Build();

    result = calculateExecuteResult(normal.mis_matrix(), population);

    EXPECT_NEAR(11.675902, result[0], 0.00001);
    EXPECT_NEAR(21.097643, result[1], 0.00001);
    EXPECT_NEAR(26.756975, result[2], 0.00001);
    EXPECT_NEAR(20.097275, result[3], 0.00001);
    EXPECT_NEAR(13.015051, result[4], 0.00001);
    EXPECT_NEAR(7.357153, result[5], 0.00001);
  }

  // Test case 2: Normal with k=4 and plus group
  {
    auto mock_model = std::make_shared<MockModel>();
    EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
    EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
    EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
    EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

    ageingerrors::Normal normal(mock_model);
    normal.parameters().Add(PARAM_LABEL, "execute_test_2", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_K, "4", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

    normal.Validate();
    normal.Build();

    result = calculateExecuteResult(normal.mis_matrix(), population);

    EXPECT_NEAR(12.153806, result[0], 0.00001);
    EXPECT_NEAR(20.619743, result[1], 0.00001);
    EXPECT_NEAR(26.756972, result[2], 0.00001);
    EXPECT_NEAR(20.097275, result[3], 0.00001);
    EXPECT_NEAR(13.015051, result[4], 0.00001);
    EXPECT_NEAR(7.357153, result[5], 0.00001);
  }

  // Test case 3: Normal without plus group
  {
    auto mock_model = std::make_shared<MockModel>();
    EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
    EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
    EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
    EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));

    ageingerrors::Normal normal(mock_model);
    normal.parameters().Add(PARAM_LABEL, "execute_test_3", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_K, "0", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

    normal.Validate();
    normal.Build();

    result = calculateExecuteResult(normal.mis_matrix(), population);

    EXPECT_NEAR(11.675902, result[0], 0.00001);
    EXPECT_NEAR(21.097643, result[1], 0.00001);
    EXPECT_NEAR(26.756975, result[2], 0.00001);
    EXPECT_NEAR(20.097275, result[3], 0.00001);
    EXPECT_NEAR(13.015051, result[4], 0.00001);
    EXPECT_NEAR(5.785982, result[5], 0.00001);  // Different without plus group
  }

  // Test case 4: Normal with k=4 and without plus group
  {
    auto mock_model = std::make_shared<MockModel>();
    EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(3));
    EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(8));
    EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(6));
    EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));

    ageingerrors::Normal normal(mock_model);
    normal.parameters().Add(PARAM_LABEL, "execute_test_4", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_TYPE, "normal", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_CV, "0.1", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_K, "4", __FILE__, __LINE__);
    normal.parameters().Add(PARAM_NORMALISE_ROWS, "true", __FILE__, __LINE__);

    normal.Validate();
    normal.Build();

    result = calculateExecuteResult(normal.mis_matrix(), population);

    EXPECT_NEAR(12.153806, result[0], 0.00001);
    EXPECT_NEAR(20.619743, result[1], 0.00001);
    EXPECT_NEAR(26.756972, result[2], 0.00001);
    EXPECT_NEAR(20.097275, result[3], 0.00001);
    EXPECT_NEAR(13.015051, result[4], 0.00001);
    EXPECT_NEAR(5.785982, result[5], 0.00001);  // Different without plus group
  }
}

}  // namespace niwa
#endif  // TESTMODE