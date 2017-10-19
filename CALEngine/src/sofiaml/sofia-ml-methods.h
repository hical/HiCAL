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

  //--------------------------------------------------------------------------
  //                     Main API methods for Model Training
  //--------------------------------------------------------------------------
  // This section contains the Main API methods to call for training a model on
  // a given data set.
  //  For each method, the parameters are described as follows:
  //   training_set  an SfDataSet filled with labeled training data.
  //   learner_type  a LearnerType enum value (defined above) 
  //                   showing which learner to use.
  //   eta_type      an EtaType enum showing how to update the learning rate.
  //   lambda        regularization parameter (ignored by some LearnerTypes)
  //   c             capacity parameter (ignored by some LearnerTypes)
  //   num_iters     number of stochastic steps to take.

  // We currently support the following learners.
  enum LearnerType {
    LOGREG_PEGASOS,  // Logistic Regression using Pegasos projection, and lambda as regularization parameter.
  };

  // Learning rate Eta may be set in different ways.
  enum EtaType {
    BASIC_ETA,  // On step i, eta = 1000 / (1000 + i)
    PEGASOS_ETA,  // On step i, eta = 1.0 / (lambda * i) 
    CONSTANT  // Use constant eta = 0.02 for all steps.
  };


  // Trains a model w over training_set, using learner_type and eta_type learner with
  // given parameters.  For each iteration, samples one positive example uniformly at
  // random from the set of all positives, and samples one negative example uniformly
  // at random from the set of all negatives.  We then take a rank step of the difference
  // of these two vectors.  This optimizes area under the ROC curve.
  void StochasticRocLoop(const std::vector<const SfSparseVector*> &positives,
                         const std::vector<const SfSparseVector*> &negatives,
			 LearnerType learner_type,
			 EtaType eta_type,
			 float lambda,
			 float c,
			 int num_iters,
			 SfWeightVector* w);

  // Takes one rank (a-b) step using the LearnerType defined by method, and returns true
  // iff the method took a gradient step (mod.
  bool OneLearnerRankStep(LearnerType method,
			  const SfSparseVector& a,
			  const SfSparseVector& b,
			  float eta,
			  float c,
			  float lambda,
			  SfWeightVector* w,
                          float y_a=INF,
                          float y_b=INF);

  //------------------------------------------------------------------------------//
  //                         LearnerType Methods                                  //
  //------------------------------------------------------------------------------//

  // Takes a single logistic regression step on vector (a-b), using pegasos
  // projection for regularization.
  bool SinglePegasosLogRegRankStep(const SfSparseVector& a,
                                   const SfSparseVector& b,
                                   float eta,
                                   float lambda,
                                   SfWeightVector* w,
                                   float y_a=INF,
                                   float y_b=INF);

  //-------------------------------------------------------------------
  //                    Non-Member Utility Functions
  //-------------------------------------------------------------------

  // Performs the PEGASOS projection step, projecting w back so that it
  // in the feasible set of solutions.
  void PegasosProjection(float lambda,
			 SfWeightVector* w);

  // Perform L2 regularization step, penalizing squared norm of
  // weight vector w.  Note that this regularization step is accomplished
  // by using w <- (1 - (eta * lambda)) * w, but we use MIN_SCALING_FACTOR
  // if (1 - (eta * lambda)) < MIN_SCALING_FACTOR to prevent numerical
  // problems.
  void L2Regularize(float eta, float lambda, SfWeightVector* w);

}  // namespace sofia_ml

#endif  // SOFIA_ML_METHODS_H__
