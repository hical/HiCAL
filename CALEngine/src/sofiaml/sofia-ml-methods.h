//===========================================================================//
// Copyright 2009 Google Inc.                                                //
//                                                                           //
// Licensed under the Apache License, Version 2.0 (the "License");           //
// you may not use this file except in compliance with the License.          //
// You may obtain a copy of the License at                                   //
//                                                                           //
//      http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                           //
// Unless required by applicable law or agreed to in writing, software       //
// distributed under the License is distributed on an "AS IS" BASIS,         //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
// See the License for the specific language governing permissions and       //
// limitations under the License.                                            //
//===========================================================================//
//
// sofia-ml-methods.h
//
// Author: D. Sculley
// dsculley@google.com or dsculley@cs.tufts.edu
//
// Non-member functions for training and applying classifiers in the
// sofia-ml suite.  This is the main API that callers will use.

#ifndef SOFIA_ML_METHODS_H__
#define SOFIA_ML_METHODS_H__

#include "sf-sparse-vector.h"
#include "sf-weight-vector.h"

namespace sofia_ml {
  const float INF = -1e18;

  // Trains a model w over training_set, using learner_type and eta_type learner with
  // given parameters.  For each iteration, samples one positive example uniformly at
  // random from the set of all positives, and samples one negative example uniformly
  // at random from the set of all negatives.  We then take a rank step of the difference
  // of these two vectors.  This optimizes area under the ROC curve.
  void StochasticRocLoop(const std::vector<const SfSparseVector*> &positives,
                         const std::vector<const SfSparseVector*> &negatives,
			 float lambda,
			 float c,
			 int num_iters,
			 SfWeightVector* w);

}  // namespace sofia_ml

#endif  // SOFIA_ML_METHODS_H__
