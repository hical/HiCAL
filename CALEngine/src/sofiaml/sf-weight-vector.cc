//==========================================================================//
// Copyright 2009 Google Inc.                                               //
//                                                                          // 
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//      http://www.apache.org/licenses/LICENSE-2.0                          //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
//==========================================================================//
//
// sf-weight-vector.cc
//
// Author: D. Sculley
// dsculley@google.com or dsculley@cs.tufts.edu
//
// Implementation of sf-weight-vector.h

#include <cmath>
#include <iostream>
#include <sstream>
#include <string>

#include "sf-weight-vector.h"

//----------------------------------------------------------------//
//---------------- SfWeightVector Public Methods ----------------//
//----------------------------------------------------------------//

SfWeightVector::SfWeightVector(int dimensionality) 
  : scale_(1.0),
    squared_norm_(0.0),
    dimensions_(dimensionality) {
  if (dimensions_ <= 0) {
    std::cerr << "Illegal dimensionality of weight vector less than 1."
	      << std::endl
	      << "dimensions_: " << dimensions_ << std::endl;
    exit(1);
  }

  weights_ = new float[dimensions_];
  if (weights_ == NULL) {
    std::cerr << "Not enough memory for weight vector of dimension: " 
	      <<  dimensions_ << std::endl;
    exit(1);
  }
  for (int i = 0; i < dimensions_; ++i) {
    weights_[i] = 0;
  }
}

SfWeightVector::~SfWeightVector() {
  delete[] weights_;
}

vector<float> SfWeightVector::AsFloatVector() {
  ScaleToOne();
  return vector<float> (weights_, weights_ + dimensions_);
}

float SfWeightVector::InnerProduct(const SfSparseVector& x,
				    float x_scale) const {
  float inner_product = 0.0;
  for (int i = 0; i < x.NumFeatures(); ++i) {
    /* std::cout<<"++ "<<i<<" "<<x.FeatureAt(i)<<" "<<x.ValueAt(i)<<std::endl; */
    inner_product += weights_[x.FeatureAt(i)] * x.ValueAt(i);
    /* std::cout<<"-- "<<i<<" "<<x.FeatureAt(i)<<" "<<x.ValueAt(i)<<std::endl; */
  }
  inner_product *= x_scale;
  inner_product *= scale_;
  return inner_product;
}

float SfWeightVector::InnerProductOnDifference(const SfSparseVector& a,
					       const SfSparseVector& b,
					       float x_scale) const {
  //   <x_scale * (a - b), w>
  // = <x_scale * a - x_scale * b, w>
  // = <x_scale * a, w> + <-1.0 * x_scale * b, w>

  float inner_product = 0.0;
  inner_product += InnerProduct(a, x_scale);
  inner_product += InnerProduct(b, -1.0 * x_scale);
  return inner_product;
}

void SfWeightVector::AddVector(const SfSparseVector& x, float x_scale) {
  if (x.FeatureAt(x.NumFeatures() - 1) > dimensions_) {
    std::cerr << "Feature " << x.FeatureAt(x.NumFeatures() - 1) 
	      << " exceeds dimensionality of weight vector: " 
	      << dimensions_ << std::endl;
    std::cerr << x.AsString() << std::endl;
    exit(1);
  }

  float inner_product = 0.0;
  for (int i = 0; i < x.NumFeatures(); ++i) {
    float this_x_value = x.ValueAt(i) * x_scale;
    int this_x_feature = x.FeatureAt(i);
    inner_product += weights_[this_x_feature] * this_x_value;
    weights_[this_x_feature] += this_x_value / scale_;
  }
  squared_norm_ += x.GetSquaredNorm() * x_scale * x_scale +
    (2.0 * scale_ * inner_product); 
}

void SfWeightVector::ScaleBy(double scaling_factor) {
  // Take care of any numerical difficulties.
  if (scale_ < 0.00000000001) ScaleToOne();

  // Do the scaling.
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


//-----------------------------------------------------------------//
//---------------- SfWeightVector Private Methods ----------------//
//-----------------------------------------------------------------//

void SfWeightVector::ScaleToOne() {
  for (int i = 0; i < dimensions_; ++i) {
    weights_[i] *= scale_;
  }
  scale_ = 1.0;
}
