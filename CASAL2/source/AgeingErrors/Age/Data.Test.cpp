/**
 * @file Data.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/03/27
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "Data.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../Model/Factory.h"
#include "../../TestResources/MockClasses/Model.h"

namespace niwa {

using ::testing::_;
using ::testing::Return;

/**
 * Test the misclassification matrix generation for Data ageing error
 */
TEST(AgeingErrors, Data_MisMatrixGeneration) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(2));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Data data(mock_model);
  data.parameters().Add(PARAM_LABEL, "test_data", __FILE__, __LINE__);
  data.parameters().Add(PARAM_TYPE, "data", __FILE__, __LINE__);

  // Create and add the data table
  parameters::table::Table* table     = data.parameters().GetTable(PARAM_DATA);
  vector<vector<string>>    test_data = {{"0.9", "0.1", "0.0", "0.0", "0.0"},
                                         {"0.1", "0.8", "0.1", "0.0", "0.0"},
                                         {"0.0", "0.1", "0.8", "0.1", "0.0"},
                                         {"0.0", "0.0", "0.1", "0.8", "0.1"},
                                         {"0.0", "0.0", "0.0", "0.1", "0.9"}};
  table->set_data(test_data);
  data.Validate();
  data.Build();

  // Get the matrix
  auto matrix = data.mis_matrix();

  // Age spread should be (max_age - min_age) + 1 = (6 - 2) + 1 = 5
  ASSERT_EQ(5u, matrix.size());

  // Verify matrix values match our input data
  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(5u, matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      double expected_value = std::stod(test_data[i][j]);
      EXPECT_DOUBLE_EQ(expected_value, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be " << expected_value;
    }
  }
}

/**
 * Test the misclassification matrix validation for row sums
 */
TEST(AgeingErrors, Data_RowSumValidation) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(2));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  // Create test data with proper row sums (each row sums to 1.0)
  ageingerrors::Data valid_data_object(mock_model);
  valid_data_object.parameters().Add(PARAM_LABEL, "test_valid_data", __FILE__, __LINE__);
  valid_data_object.parameters().Add(PARAM_TYPE, "data", __FILE__, __LINE__);

  parameters::table::Table* valid_table = valid_data_object.parameters().GetTable(PARAM_DATA);
  vector<vector<string>>    valid_data  = {{"0.9", "0.1", "0.0", "0.0", "0.0"},
                                           {"0.1", "0.8", "0.1", "0.0", "0.0"},
                                           {"0.0", "0.1", "0.8", "0.1", "0.0"},
                                           {"0.0", "0.0", "0.1", "0.8", "0.1"},
                                           {"0.0", "0.0", "0.0", "0.1", "0.9"}};
  valid_table->set_data(valid_data);
  valid_data_object.Validate();
  valid_data_object.Build();

  // The matrix should be correctly created
  auto valid_matrix = valid_data_object.mis_matrix();
  ASSERT_EQ(5u, valid_matrix.size());

  // Set a custom tolerance to ensure the error is caught
  ageingerrors::Data invalid_data_object(mock_model);
  invalid_data_object.parameters().Add(PARAM_LABEL, "test_invalid_data", __FILE__, __LINE__);
  invalid_data_object.parameters().Add(PARAM_TYPE, "data", __FILE__, __LINE__);
  invalid_data_object.parameters().Add(PARAM_TOLERANCE, "0.001", __FILE__, __LINE__);

  // Create test data with invalid row sums (row doesn't sum to 1.0)
  parameters::table::Table* invalid_table = invalid_data_object.parameters().GetTable(PARAM_DATA);
  vector<vector<string>>    invalid_data  = {{"0.9", "0.1", "0.0", "0.0", "0.0"},
                                             {"0.2", "0.8", "0.1", "0.0", "0.0"},  // This row sums to 1.1
                                             {"0.0", "0.1", "0.8", "0.1", "0.0"},
                                             {"0.0", "0.0", "0.1", "0.8", "0.1"},
                                             {"0.0", "0.0", "0.0", "0.1", "0.9"}};
  invalid_table->set_data(invalid_data);
  // The Build should detect the row sum error, but since we can't check LOG_ERROR in tests,
  // we'll just validate and build it without asserting on the error
  EXPECT_NO_THROW(invalid_data_object.Validate());
  // Note: In a real application, Build would log an error due to row sum > 1.0 + tolerance
  EXPECT_THROW(invalid_data_object.Build(), std::string);
}

/**
 * Test Data with different matrix dimensions
 */
TEST(AgeingErrors, Data_DifferentMatrixSize) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(1));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(3));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(true));

  ageingerrors::Data data(mock_model);
  data.parameters().Add(PARAM_LABEL, "test_small_data", __FILE__, __LINE__);
  data.parameters().Add(PARAM_TYPE, "data", __FILE__, __LINE__);

  // Create and add a smaller data table (3x3)
  parameters::table::Table* table     = data.parameters().GetTable(PARAM_DATA);
  vector<vector<string>>    test_data = {{"0.9", "0.1", "0.0"}, {"0.1", "0.8", "0.1"}, {"0.0", "0.1", "0.9"}};
  table->set_data(test_data);

  data.Validate();
  data.Build();

  // Get the matrix
  auto matrix = data.mis_matrix();

  // Age spread should be (max_age - min_age) + 1 = (3 - 1) + 1 = 3
  ASSERT_EQ(3u, matrix.size());

  // Verify matrix values match our input data
  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(3u, matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      double expected_value = std::stod(test_data[i][j]);
      EXPECT_DOUBLE_EQ(expected_value, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be " << expected_value;
    }
  }
}

/**
 * Test Data with no plus group
 */
TEST(AgeingErrors, Data_NoPlusGroup) {
  auto mock_model = std::make_shared<MockModel>();
  EXPECT_CALL(*mock_model, min_age()).WillRepeatedly(Return(2));
  EXPECT_CALL(*mock_model, max_age()).WillRepeatedly(Return(6));
  EXPECT_CALL(*mock_model, age_spread()).WillRepeatedly(Return(5));
  EXPECT_CALL(*mock_model, age_plus()).WillRepeatedly(Return(false));  // No plus group

  ageingerrors::Data data(mock_model);
  data.parameters().Add(PARAM_LABEL, "test_no_plus_group", __FILE__, __LINE__);
  data.parameters().Add(PARAM_TYPE, "data", __FILE__, __LINE__);

  // Create and add the data table where rows don't all sum to 1.0 (valid for no plus group)
  parameters::table::Table* table     = data.parameters().GetTable(PARAM_DATA);
  vector<vector<string>>    test_data = {
      {"0.9", "0.1", "0.0", "0.0", "0.0"},
      {"0.1", "0.8", "0.1", "0.0", "0.0"},
      {"0.0", "0.1", "0.7", "0.1", "0.0"},  // Row sum < 1.0 is okay with no plus group
      {"0.0", "0.0", "0.1", "0.8", "0.1"},
      {"0.0", "0.0", "0.0", "0.1", "0.8"}  // Last row < 1.0 is valid with no plus group
  };
  table->set_data(test_data);

  data.Validate();
  data.Build();

  // Get the matrix
  auto matrix = data.mis_matrix();

  // Age spread should be (max_age - min_age) + 1 = (6 - 2) + 1 = 5
  ASSERT_EQ(5u, matrix.size());

  // Verify matrix values match our input data
  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(5u, matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      double expected_value = std::stod(test_data[i][j]);
      EXPECT_DOUBLE_EQ(expected_value, matrix[i][j]) << "Element at position [" << i << "][" << j << "] should be " << expected_value;
    }
  }
}

}  // namespace niwa
#endif  // TESTMODE