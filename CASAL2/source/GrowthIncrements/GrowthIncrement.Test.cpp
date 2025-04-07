/**
 * @file GrowthIncrement.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/01
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

#include "GrowthIncrement.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <numeric>

#include "../Model/Managers.h"
#include "../Model/Model.h"
#include "../TestResources/Models/UnsexedLengthBased.h"
#include "../TestResources/TestFixtures/InternalEmptyLengthModel.h"
#include "Length/Basic.h"
#include "Manager.h"

namespace niwa {
using niwa::testfixtures::InternalEmptyLengthModel;
using std::cout;
using std::endl;

/*
MATRIX Growth_years_same<DVM>::make_matrix(Basic_state<DVM>& s, const VECTOR& g, const VECTOR& l,
                                           const DOUBLE& cv, const DOUBLE& minsigma,
                                           const std::string growth_type_name){
  // This function is used to make a transition matrix for the basic growth method.
  // It is called by Growth_years_same<DVM>::Growth_years_same()
  DEBUG2("Growth_years_same::make_matrix");
  DOUBLE mu, stdev, sum_so_far;
  dvector l_c(1,s.n_cols);
  for (int i=1; i<s.n_cols; i++){
    l_c[i] = (s.class_mins[i]+s.class_mins[i+1])*0.5;
  }
  if (!s.plus_group){
    l_c[s.n_cols] = (s.class_mins[s.n_cols]+s.class_mins[s.n_cols+1])*0.5;
  }

  MATRIX transition(1,s.n_cols,1,s.n_cols);
  transition.initialize();
  for (int i=1; i<=s.n_cols; i++){
    // set the ith row, which is the transitions from the ith column.
    for (int j=1; j<i; j++){
      transition[i][j] = 0; // no negative growth
    }
    if (s.plus_group && i==s.n_cols){
      transition[i][i] = 1; // plus group can't grow any more
    } else {
      //mu = g[1] + (g[2]-g[1])*(l_c[i]-l[1])/(l[2]-l[1]);
      //stdev = fmax(cv*mu, minsigma);
      if(growth_type_name=="basic"){
        mu = g[1] + (g[2]-g[1])*(l_c[i]-l[1])/(l[2]-l[1]);
        stdev = fmax(cv*mu, minsigma);
      } else if (growth_type_name=="exponential"){
        mu = g[1] * pow(g[2]/g[1],(l_c[i]-l[1])/(l[2]-l[1]));
        stdev = fmax(cv*mu, minsigma);
      } else fatal("Unknown growth type: " + growth_type_name);
      transition[i][i] = pnorm<DOUBLE>(s.class_mins[i+1]-l_c[i],mu,stdev);
      sum_so_far = transition[i][i];
      for (int j=i+1;j<s.n_cols; j++){
        transition[i][j] = pnorm<DOUBLE>(s.class_mins[j+1]-l_c[i],mu,stdev) - sum_so_far;
        sum_so_far += transition[i][j];
      }
      if (s.plus_group){
        transition[i][s.n_cols] = 1 - sum_so_far;
      } else {
        transition[i][s.n_cols] = pnorm<DOUBLE>(s.class_mins[s.n_cols+1]-l_c[i],mu,stdev) - sum_so_far;
      }
    }
  }
  return transition;
}

*/

/**
 * Test growth transition matrix calculation
 */
TEST_F(InternalEmptyLengthModel, GrowthIncrement_GrowthTransitionMatrix) {
  const string growth_transition_matrix = R"(
    @growth_increment growth_model
    type basic
    l_alpha 5
    l_beta  10
    g_alpha 7
    g_beta 1
    min_sigma 2
    distribution normal
    length_weight allometric
    cv 0.0
    compatibility_option casal
    )";

  AddConfigurationLine(testresources::models::length_based_unsexed_basic_with_length_plus, "TestResources/Models/UnsexedLengthBasic.h", 111);
  AddConfigurationLine(growth_transition_matrix, __FILE__, 32);
  LoadConfiguration();
  model_->Start(RunMode::kTesting);

  auto object       = model_->managers()->growth_increment()->get("growth_model");
  auto growth_model = dynamic_cast<niwa::growthincrements::Basic*>(object);
  ASSERT_NE(nullptr, growth_model);

  // Verify matrix dimensions
  auto matrix = growth_model->get_transition_matrix();
  ASSERT_EQ(15u, matrix.size());
  for (const auto& row : matrix) {
    ASSERT_EQ(15u, row.size());
  }

  // Print the matrix for debugging purposes
  // cout << "Transition Matrix:" << endl;
  // for (const auto& row : matrix) {
  //   for (const auto& value : row) {
  //     cout << value << " ";
  //   }
  //   cout << endl;
  // }

  for (unsigned i = 0; i < matrix.size(); ++i) {
    Double line_sum = 0.0;
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      line_sum += matrix[i][j];
    }
    Double expected_sum = i < matrix.size() - 1 ? 1.0 : 1.0;
    EXPECT_NEAR(expected_sum, line_sum, 1e-10) << "Where i = " << i;
  }
};

/**
 * Test transition matrix with different CV values
 */
TEST_F(InternalEmptyLengthModel, GrowthIncrement_TransitionMatrixCV) {
  const string growth_transition_matrix = R"(
    @growth_increment growth_model
    type basic
    l_alpha 5
    l_beta  10
    g_alpha 7
    g_beta 1
    min_sigma 0.1
    distribution normal
    length_weight allometric
    cv 0.1
    compatibility_option casal
    )";

  AddConfigurationLine(testresources::models::length_based_unsexed_basic_with_length_plus, "TestResources/Models/UnsexedLengthBasic.h", 111);
  AddConfigurationLine(growth_transition_matrix, __FILE__, 81);
  LoadConfiguration();
  model_->Start(RunMode::kTesting);

  auto object       = model_->managers()->growth_increment()->get("growth_model");
  auto growth_model = dynamic_cast<niwa::growthincrements::Basic*>(object);
  ASSERT_NE(nullptr, growth_model);

  // Verify matrix dimensions
  auto matrix = growth_model->get_transition_matrix();
  ASSERT_EQ(15u, matrix.size());
  for (const auto& row : matrix) {
    ASSERT_EQ(15u, row.size());
  }

  // Print the matrix for debugging purposes
  // Define the expected matrix
  std::vector<std::vector<Double>> expected_matrix = {
      {5.16843e-10, 1.78271e-09, 9.46819e-09, 5.91241e-08, 4.44719e-07, 4.11167e-06, 4.66809e-05, 0.000609018, 0.00737812, 0.0563936, 0.201396, 0.339766, 0.271316, 0.10324,
       0.0198493},
      {0, 5.63257e-10, 2.48825e-09, 1.67515e-08, 1.39392e-07, 1.48222e-06, 2.05105e-05, 0.000352257, 0.00596373, 0.0603917, 0.241575, 0.38339, 0.241575, 0.0603917, 0.00633814},
      {0, 0, 6.28802e-10, 3.77496e-09, 3.48694e-08, 4.29787e-07, 7.38445e-06, 0.000174345, 0.00447602, 0.0650864, 0.296718, 0.420315, 0.186746, 0.025177, 0.00129967},
      {0, 0, 0, 7.27597e-10, 6.4939e-09, 9.34398e-08, 2.03074e-06, 6.92545e-05, 0.00301023, 0.070787, 0.374011, 0.433756, 0.112026, 0.00618726, 0.000150879},
      {0, 0, 0, 0, 8.91097e-10, 1.36999e-08, 3.83304e-07, 1.99436e-05, 0.00170579, 0.0781106, 0.482049, 0.395501, 0.0419086, 0.000695474, 8.78851e-06},
      {0, 0, 0, 0, 0, 1.20361e-09, 4.17533e-08, 3.50959e-06, 0.000725557, 0.0884677, 0.629007, 0.275459, 0.00631078, 2.71279e-05, 2.24095e-07},
      {0, 0, 0, 0, 0, 0, 1.96895e-09, 2.7637e-07, 0.00018192, 0.105622, 0.788391, 0.105622, 0.00018192, 2.7637e-07, 1.96895e-09},
      {0, 0, 0, 0, 0, 0, 0, 5.10475e-09, 1.50257e-05, 0.142206, 0.851441, 0.00633772, 4.18505e-07, 4.44871e-10, 3.38574e-12},
      {0, 0, 0, 0, 0, 0, 0, 0, 7.08918e-08, 0.265828, 0.73417, 1.20838e-06, 1.79393e-11, 2.30926e-14, 2.22045e-16},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0.841124, 0.158876, 5.89614e-11, 2.22045e-15, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4.37261e-12, 5.55112e-16, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1.11022e-16, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
  };

  // Compare the actual matrix to the expected matrix
  ASSERT_EQ(expected_matrix.size(), matrix.size());
  for (unsigned i = 0; i < matrix.size(); ++i) {
    ASSERT_EQ(expected_matrix[i].size(), matrix[i].size());
    for (unsigned j = 0; j < matrix[i].size(); ++j) {
      EXPECT_NEAR(expected_matrix[i][j], matrix[i][j], 1e-5) << "Mismatch at (" << i << ", " << j << ")";
    }
  }
}

}  // namespace niwa
#endif  // TESTMODE