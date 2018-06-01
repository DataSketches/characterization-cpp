/*
 * Copyright 2018, Oath Inc. Licensed under the terms of the
 * Apache License 2.0. See LICENSE file at the project root for terms.
 */

#ifndef KLL_SKETCH_TIMING_PROFILE_H_
#define KLL_SKETCH_TIMING_PROFILE_H_

namespace sketches {

class kll_sketch_timing_profile {
public:
  kll_sketch_timing_profile();
  virtual ~kll_sketch_timing_profile();
  virtual void run();
};

} /* namespace sketches */

#endif /* KLL_SKETCH_TIMING_PROFILE_H_ */
