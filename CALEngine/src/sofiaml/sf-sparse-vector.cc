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
//
// Implementation of sf-sparse-vector.h

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include "sf-sparse-vector.h"

//----------------------------------------------------------------//
//---------------- SfSparseVector Public Methods ----------------//
//----------------------------------------------------------------//

SfSparseVector::SfSparseVector(const vector<FeatureValuePair> &feature_vector)
{
    SetBias();
    for(auto feature_value_pair: feature_vector)
        PushPair(feature_value_pair.id_, feature_value_pair.value_);
}

SfSparseVector::SfSparseVector(string doc_id, const vector<FeatureValuePair> &feature_vector)
  : doc_id(doc_id)
{
    SetBias();
    for(auto feature_value_pair: feature_vector)
        PushPair(feature_value_pair.id_, feature_value_pair.value_);
}

void SfSparseVector::PushPair(uint32_t id, float value) {
  if (id > 0 && NumFeatures() > 0 && id <= FeatureAt(NumFeatures() - 1) ) {
    std::cerr << id << " vs. " << FeatureAt(NumFeatures() - 1) << std::endl;
    DieFormat("Features not in ascending sorted order.");
  }

  FeatureValuePair feature_value_pair;
  feature_value_pair.id_ = id;
  feature_value_pair.value_ = value;
  features_.push_back(feature_value_pair);
  squared_norm_ += value * value;
}

//----------------------------------------------------------------//
//--------------- SfSparseVector Private Methods ----------------//
//----------------------------------------------------------------//

void SfSparseVector::DieFormat(const string& reason) {
  std::cerr << "Wrong format for input data:\n  " << reason << std::endl;
  exit(1);
}
