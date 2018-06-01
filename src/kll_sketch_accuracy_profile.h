/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef KLL_SKETCH_ACCURACY_PROFILE_H_
#define KLL_SKETCH_ACCURACY_PROFILE_H_

#include "kll_accuracy_profile.h"

namespace sketches {

class kll_sketch_accuracy_profile: public kll_accuracy_profile {
public:
  virtual double run_trial(float* values, unsigned stream_length);
};

} /* namespace sketches */

#endif /* KLL_SKETCH_ACCURACY_PROFILE_H_ */
