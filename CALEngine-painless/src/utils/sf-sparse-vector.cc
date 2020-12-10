//================================================================================//
// Copyright 2009 Google Inc. //
//                                                                                //
// Licensed under the Apache License, Version 2.0 (the "License"); // you may
// not use this file except in compliance with the License.               // You
// may obtain a copy of the License at                                        //
//                                                                                //
//      http://www.apache.org/licenses/LICENSE-2.0 //
//                                                                                //
// Unless required by applicable law or agreed to in writing, software //
// distributed under the License is distributed on an "AS IS" BASIS, // WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.       // See
// the License for the specific language governing permissions and            //
// limitations under the License. //
//================================================================================//
//
//
// Implementation of sf-sparse-vector.h

#include "utils/sf-sparse-vector.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>

//----------------------------------------------------------------//
//---------------- SfSparseVector Public Methods ----------------//
//----------------------------------------------------------------//

SfSparseVector::SfSparseVector(const vector<FeatureValuePair> &features) {
    for (auto &fvp : features) PushPair(fvp);
}

void SfSparseVector::PushPair(FeatureValuePair feature_value_pair) {
    if (feature_value_pair.id_ > 0 && NumFeatures() > 0 &&
        feature_value_pair.id_ <= FeatureAt(NumFeatures() - 1)) {
        std::cerr << feature_value_pair.id_ << " vs. "
                  << FeatureAt(NumFeatures() - 1) << std::endl;
        DieFormat("Features not in ascending sorted order.");
    }

    features_.push_back(feature_value_pair);
    squared_norm_ += feature_value_pair.value_ * feature_value_pair.value_;
}

//----------------------------------------------------------------//
//--------------- SfSparseVector Private Methods ----------------//
//----------------------------------------------------------------//

void SfSparseVector::DieFormat(const string &reason) {
    std::cerr << "Wrong format for input data:\n  " << reason << std::endl;
    exit(1);
}
