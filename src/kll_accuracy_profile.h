/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef KLL_ACCURACY_PROFILE_H_
#define KLL_ACCURACY_PROFILE_H_

namespace datasketches {

class kll_accuracy_profile {
public:
  virtual void run();
  virtual double run_trial(float* values, unsigned stream_length) = 0;
};

} /* namespace datasketches */

#endif /* KLL_ACCURACY_PROFILE_H_ */
