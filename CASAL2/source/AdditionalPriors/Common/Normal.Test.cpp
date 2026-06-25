/**
 * @file Normal.Test.cpp
 * @author Casal2 Team
 * @section LICENSE
 *
 * Copyright Casal2 Project 2024 - https://github.com/Casal2/
 */
#ifdef TESTMODE

// Headers
#include "Normal.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

#include "../../Model/Model.h"
#include "../../TestResources/MockClasses/Model.h"

// Namespaces
namespace niwa {
namespace additionalpriors {

/*
 * Mock class to make testing easier
 */
class MockNormal : public Normal {
public:
  MockNormal(shared_ptr<Model> model, double mu, double cv, double* parameter) : Normal(model) {
    mu_          = mu;
    cv_          = cv;
    addressable_ = parameter;
  }
};

/**
 * Test the results are correct
 */
TEST(AdditionalPriors, Normal) {
  // layout is mu, cv, value, expected_score
  vector<vector<double>> values = {{4.3, 0.05, 5.46, 14.5548945375879}, {2.3, 0.4, 6.24, 9.17036862003781}};

  shared_ptr<Model> model = shared_ptr<Model>(new Model());
  for (auto line : values) {
    MockNormal normal(model, line[0], line[1], &line[2]);
    EXPECT_NEAR(line[3], normal.GetScore(), 1e-10);
  }
}

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* ifdef TESTMODE */
