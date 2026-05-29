/**
 * @file AR1.Test.cpp
 * @author Casal2
 * @date 2026/05/29
 * @section LICENSE
 *
 * Copyright Casal2 Project 2026 - https://github.com/Casal2/
 */
#ifdef TESTMODE

// headers
#include "AR1.h"

#include <gtest/gtest.h>

#include "../../Model/Model.h"

// namespaces
namespace niwa {
namespace additionalpriors {

class MockAR1 : public AR1 {
public:
  MockAR1(shared_ptr<Model> model, Double rho, Double sigma, Double mean, Double multiplier, bool log_scale, vector<Double>* values) : AR1(model) {
    rho_                = rho;
    sigma_              = sigma;
    mean_               = mean;
    multiplier_         = multiplier;
    log_scale_          = log_scale;
    addressable_vector_ = values;
  }
};

TEST(AdditionalPriors, AR1_Score) {
  shared_ptr<Model> model = shared_ptr<Model>(new Model());

  {
    vector<Double> values = {1.0, 1.4, 1.7};
    MockAR1        ar1(model, 0.6, 0.5, 0.0, 2.0, false, &values);
    EXPECT_DOUBLE_EQ(5.5184, ar1.GetScore());
  }

  {
    vector<Double> values = {1.2, 1.1, 0.9};
    MockAR1        ar1(model, 0.5, 0.2, 1.0, 1.0, false, &values);
    EXPECT_DOUBLE_EQ(0.28125, ar1.GetScore());
  }
}

} /* namespace additionalpriors */
} /* namespace niwa */

#endif /* TESTMODE */
