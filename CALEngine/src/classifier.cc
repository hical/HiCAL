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


#include "classifier.h"
#define MIN_SCALING_FACTOR 0.0000001
void LRPegasosClassifier::train(const std::vector<const SfSparseVector*> &positives,
                                const std::vector<const SfSparseVector*> &negatives,
                                SfWeightVector* w) {
    // For each step, randomly sample one positive and one negative and
    // take a pairwise gradient step.
    for (int i = 1; i <= num_iters; ++i) {

        float eta = 1.0 / (lambda * i);
        const SfSparseVector& a =
            *positives[RandInt(positives.size())];
        const SfSparseVector& b =
            *negatives[RandInt(negatives.size())];

        float y = 1.0;
        float loss = y / (1 + exp(y * w->InnerProductOnDifference(a, b)));

        // L2Regularize
        float scaling_factor = 1.0 - (eta * lambda);
        if (scaling_factor > MIN_SCALING_FACTOR) {
            w->ScaleBy(1.0 - (eta * lambda));
        } else {
            w->ScaleBy(MIN_SCALING_FACTOR);
        }

        w->AddVector(a, (eta * loss));
        w->AddVector(b, (-1.0 * eta * loss));

        // Pegasos Projection
        float projection_val = 1 / sqrt(lambda * w->GetSquaredNorm());
        if (projection_val < 1.0) {
            w->ScaleBy(projection_val);
        }
    }

}
