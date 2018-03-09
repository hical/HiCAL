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

float InnerProduct(const SfSparseVector& x, vector<float> &w, double &scale_) {
    float inner_product = 0.0;
    for (int i = 0; i < x.NumFeatures(); ++i) {
        inner_product += w[x.FeatureAt(i)] * x.ValueAt(i);
    }
    inner_product *= scale_;
    return inner_product;
}

float InnerProductOnDifference(const SfSparseVector& a,
                               const SfSparseVector& b,
                               vector<float> &w, double &scale_) {
    return InnerProduct(a, w, scale_) - InnerProduct(b, w, scale_);
}

void ScaleToOne(vector<float> &weights, double &scale_) {
    for (int i = 0; i < weights.size(); ++i) {
        weights[i] *= scale_;
    }
    scale_ = 1.0;
}

void ScaleBy(vector<float> &w, double scaling_factor, double &scale_, double &squared_norm_) {
  // Take care of any numerical difficulties.
  if (scale_ < 0.00000000001) ScaleToOne(w, scale_);
  squared_norm_ *= (scaling_factor * scaling_factor);

  if (scaling_factor > 0.0) {
    scale_ *= scaling_factor;
  } else {
    std::cerr << "Error: scaling weight vector by non-positive value!\n " 
	      << "This can cause numerical errors in PEGASOS projection.\n "
	      << "This is likely due to too large a value of eta * lambda.\n "
	      << std::endl;
    exit(1);
  }
}

void AddVector(const SfSparseVector& x, float x_scale, vector<float> &w, double &scale_, double &squared_norm_) {
  float inner_product = 0.0;

  for (int i = 0; i < x.NumFeatures(); ++i) {
    float this_x_value = x.ValueAt(i) * x_scale;
    const int this_x_feature = x.FeatureAt(i);
    inner_product += w[this_x_feature] * this_x_value;
    w[this_x_feature] += this_x_value / scale_;
  }

  squared_norm_ += x.GetSquaredNorm() * x_scale * x_scale +
   (2.0 * scale_ * inner_product); 
}

vector<float> LRPegasosClassifier::train(const std::vector<const SfSparseVector*> &positives,
                                const std::vector<const SfSparseVector*> &negatives,
                                int dimensionality) {
    // For each step, randomly sample one positive and one negative and
    // take a pairwise gradient step.
    vector<float> w(dimensionality);
    std::fill(w.begin(), w.end(), 0.0);
    double scale_ = 1.0, squared_norm_ = 0.0;

    for (int i = 1; i <= num_iters; ++i) {

        float eta = 1.0 / (lambda * i);
        const SfSparseVector& a =
            *positives[RandInt(positives.size())];
        const SfSparseVector& b =
            *negatives[RandInt(negatives.size())];

        float y = 1.0;
        float loss = y / (1 + exp(y * InnerProductOnDifference(a, b, w, scale_)));

        // L2Regularize
        float scaling_factor = 1.0 - (eta * lambda);
        if (scaling_factor > MIN_SCALING_FACTOR) {
            ScaleBy(w, 1.0 - (eta * lambda), scale_, squared_norm_);
        } else {
            ScaleBy(w, MIN_SCALING_FACTOR, scale_, squared_norm_);
        }

        AddVector(a, (eta * loss), w, scale_, squared_norm_);
        AddVector(b, (-1.0 * eta * loss), w, scale_, squared_norm_);

        // Pegasos Projection
        float projection_val = 1 / sqrt(lambda * squared_norm_);
        if (projection_val < 1.0) {
            ScaleBy(w, projection_val, scale_, squared_norm_);
        }

    }
    ScaleToOne(w, scale_);
    return w;
}
