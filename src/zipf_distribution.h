/*
 * Copyright 2019, Verizon Media.
 * Licensed under the terms of the Apache License 2.0. See LICENSE file at the project root for terms.
 */

// Based on the following Java implementation:
// https://github.com/apache/commons-statistics/blob/master/commons-statistics-distribution/src/main/java/org/apache/commons/statistics/distribution/ZipfDistribution.java
// https://github.com/apache/commons-rng/blob/master/commons-rng-sampling/src/main/java/org/apache/commons/rng/sampling/distribution/RejectionInversionZipfSampler.java

#ifndef ZIPF_DISTRIBUTION_H_
#define ZIPF_DISTRIBUTION_H_

#include <stdexcept>
#include <random>

namespace datasketches {

class zipf_distribution {
public:
  zipf_distribution(unsigned num_elements, double exponent);
  unsigned sample();
private:
  static constexpr double TAYLOR_THRESHOLD = 1e-8;
  static constexpr double F_1_2 = 0.5;
  static constexpr double F_1_3 = 1.0 / 3.0;
  static constexpr double F_1_4 = 0.25;

  const unsigned num_elements;
  const double exponent;
  const double h_integral_x1;
  const double h_integral_num_elements;
  const double s;

  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution;

  double h(double x);
  double h_integral(double x);
  double h_integral_inverse(double x);

  static double helper1(double x);
  static double helper2(double x);
};

} /* namespace datasketches */

#endif
