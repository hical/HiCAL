//================================================================================//
// Copyright 2009 Google Inc.                                                     //
//                                                                                // 
// Licensed under the Apache License, Version 2.0 (the "License");                //
// you may not use this file except in compliance with the License.               //
// You may obtain a copy of the License at                                        //
//                                                                                //
//      http://www.apache.org/licenses/LICENSE-2.0                                //
//                                                                                //
// Unless required by applicable law or agreed to in writing, software            //
// distributed under the License is distributed on an "AS IS" BASIS,              //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.       //
// See the License for the specific language governing permissions and            //
// limitations under the License.                                                 //
//================================================================================//
//
// sofia-ml-methods.cc
//
// Author: D. Sculley
// dsculley@google.com or dsculley@cs.tufts.edu
//
// Implementation of sofia-ml-methods.h

#include "sofia-ml-methods.h"

#include <climits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <random>

// The MIN_SCALING_FACTOR is used to protect against combinations of
// lambda * eta > 1.0, which will cause numerical problems for regularization
// and PEGASOS projection.  
#define MIN_SCALING_FACTOR 0.0000001

namespace sofia_ml {
  
  // --------------------------------------------------- //
  //         Helper functions (Not exposed in API)
  // --------------------------------------------------- //

  thread_local std::mt19937 rand_generator;
  int RandInt(int num_vals) {
    std::uniform_int_distribution<int> distribution(0, num_vals-1);
    return distribution(rand_generator);
  }

  inline float GetEta (EtaType eta_type, float lambda, int i) {
    switch (eta_type) {
    case BASIC_ETA:
      return 10.0 / (i + 10.0);
      break;
    case PEGASOS_ETA:
      return 1.0 / (lambda * i);
      break;
    case CONSTANT:
      return 0.02;
      break;
    default:
      std::cerr << "EtaType " << eta_type << " not supported." << std::endl;
      exit(0);
    }
    std::cerr << "Error in GetEta, we should never get here." << std::endl;
    return 0;
  }
  
  // --------------------------------------------------- //
  //            Stochastic Loop Strategy Functions
  // --------------------------------------------------- //

  void StochasticRocLoop(const std::vector<const SfSparseVector*> &positives,
                         const std::vector<const SfSparseVector*> &negatives,
			 LearnerType learner_type,
			 EtaType eta_type,
			 float lambda,
			 float c,
			 int num_iters,
			 SfWeightVector* w) {

    // For each step, randomly sample one positive and one negative and
    // take a pairwise gradient step.
    for (int i = 1; i <= num_iters; ++i) {
      float eta = GetEta(eta_type, lambda, i);
      const SfSparseVector& pos_x =
	*positives[RandInt(positives.size())];
      const SfSparseVector& neg_x =
	*negatives[RandInt(negatives.size())];
      OneLearnerRankStep(learner_type, pos_x, neg_x, eta, c, lambda, w, 1, -1);
    }
  }

  // --------------------------------------------------- //
  //       Single Stochastic Step Strategy Methods
  // --------------------------------------------------- //

  bool OneLearnerRankStep(LearnerType learner_type,
			  const SfSparseVector& a,
			  const SfSparseVector& b,
			  float eta,
			  float c,
			  float lambda,
			  SfWeightVector* w,
                          float y_a,
                          float y_b) {
    switch (learner_type) {
    case LOGREG_PEGASOS:
      return SinglePegasosLogRegRankStep(a, b, eta, lambda, w, y_a, y_b);
    default:
      std::cerr << "Error: learner_type " << learner_type
		<< " not supported." << std::endl;
      exit(0);
    }
  }

  // --------------------------------------------------- //
  //            Single Stochastic Step Functions
  // --------------------------------------------------- //
  
  bool SinglePegasosLogRegRankStep(const SfSparseVector& a,
				   const SfSparseVector& b,
				   float eta,
				   float lambda,
				   SfWeightVector* w,
                                   float y_a,
                                   float y_b) {
    float y = (y_a > y_b) ? 1.0 :
      (y_a < y_b) ? -1.0 : 0.0;
    float loss = y / (1 + exp(y * w->InnerProductOnDifference(a, b)));
    L2Regularize(eta, lambda, w);    

    w->AddVector(a, (eta * loss));
    w->AddVector(b, (-1.0 * eta * loss));

    PegasosProjection(lambda, w);
    return (true);
  }

  void L2Regularize(float eta, float lambda, SfWeightVector* w) {
    float scaling_factor = 1.0 - (eta * lambda);
    if (scaling_factor > MIN_SCALING_FACTOR) {
      w->ScaleBy(1.0 - (eta * lambda));  
    } else {
      w->ScaleBy(MIN_SCALING_FACTOR); 
    }
  }
  
  void PegasosProjection(float lambda, SfWeightVector* w) {
    float projection_val = 1 / sqrt(lambda * w->GetSquaredNorm());
    if (projection_val < 1.0) {
      w->ScaleBy(projection_val);
    }
  }
  
}  // namespace sofia_ml
  
