/**
 * @file Exponential.Test.cpp
 * @author  Scott Rasmussen (scott.rasmussen@zaita.com)
 * @date 2025/04/01
 * @section LICENSE
 *
 * Copyright Casal2 Project 2025 - https://github.com/Casal2/
 *
 */
#ifdef TESTMODE

// Headers
#include "Exponential.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../../TestResources/MockClasses/Managers.h"
#include "../../TestResources/MockClasses/Model.h"

// Namespaces
namespace niwa {
namespace growthincrements {
using ::testing::Return;
using ::testing::ReturnRef;

class MockExponential : public Exponential {
public:
  MockExponential(shared_ptr<Model> model, Double g_a, Double g_b, Double l_a, Double l_b) : Exponential(model) {
    g_a_ = g_a;
    g_b_ = g_b;
    l_a_ = l_a;
    l_b_ = l_b;
  }
  Double Mockget_value(Double length) { return this->get_mean_increment(length); }
};

/**
 * Test the results of our exponential growth increment model
#' exponential_growth
#' @param length
#' @param ga
#' @param gb
#' @param la
#' @param lb
#' @return mean length increment
#'
exponential_growth = function(length, ga, gb, la, lb) {
  ga * (gb / ga)^((length - la) / (lb - la))
}
length_bins = 1:43
paste(length_bins, collapse = ", ")
la = 20
lb = 40
ga = 10
gb = 1
result = vector()
for(i in 1:length(length_bins))
  result[i] = exponential_growth(length_bins[i], ga, gb, la, lb)
paste(result, collapse = ", ")
 */
TEST(GrowthIncrements, Exponential) {
  shared_ptr<MockModel> model = shared_ptr<MockModel>(new MockModel());
  vector<double>        lengths
      = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43};
  vector<double>  result = {89.12509381, 79.43282347, 70.79457844, 63.09573445, 56.23413252, 50.11872336, 44.66835922, 39.81071706, 35.48133892, 31.6227766,  28.18382931,
                            25.11886432, 22.38721139, 19.95262315, 17.7827941,  15.84893192, 14.12537545, 12.58925412, 11.22018454, 10,          8.912509381, 7.943282347,
                            7.079457844, 6.309573445, 5.623413252, 5.011872336, 4.466835922, 3.981071706, 3.548133892, 3.16227766,  2.818382931, 2.511886432, 2.238721139,
                            1.995262315, 1.77827941,  1.584893192, 1.412537545, 1.258925412, 1.122018454, 1,           0.891250938, 0.794328235, 0.707945784};
  MockExponential exponential(model, 10.0, 1.0, 20.0, 40.0);
  for (unsigned i = 0; i < lengths.size(); i++) {
    EXPECT_NEAR(exponential.Mockget_value(lengths[i]), result[i], 1e-3) << " i " << i << " get _value = " << exponential.Mockget_value(lengths[i]) << " result " << result[i];
  }
}

// Additional test with different parameter values
TEST(GrowthIncrements, ExponentialAlternateValues) {
  shared_ptr<MockModel> model   = shared_ptr<MockModel>(new MockModel());
  vector<double>        lengths = {10, 15, 20, 25, 30, 35, 40};
  // Expected values for g_a = 8.0, g_b = 2.0, l_a = 15.0, l_b = 35.0
  vector<double> result = {11.3137085, 8, 5.656854249, 4, 2.828427125, 2, 1.414213562};

  MockExponential exponential(model, 8.0, 2.0, 15.0, 35.0);
  for (unsigned i = 0; i < lengths.size(); i++) {
    EXPECT_NEAR(exponential.Mockget_value(lengths[i]), result[i], 1e-3) << " i " << i << " get _value = " << exponential.Mockget_value(lengths[i]) << " result " << result[i];
  }
}

} /* namespace growthincrements */
} /* namespace niwa */

#endif