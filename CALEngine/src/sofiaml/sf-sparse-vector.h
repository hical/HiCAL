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
// sf-sparse-vector.h
//
// Author: D. Sculley, December 2008
// dsculley@google.com or dsculley@cs.tufts.edu
//
// A sparse vector for use with sofia-ml.  Vector elements are contained
// in an stl vector, stored in a struct containing (feature id, feature value)
// pairs.  Feature id's are assumed to be unique, sorted, and strictly positive.
// Sparse vector is assumed to be in svm-light format, with the distinction
// that the class label may be a float rather than an integer, and
// that there is an optional group id value.
//
// <label> <group id?> <feature>:<value> ... <feature><value> <#comment?>
//
// The group id is set by a <prefix>:<id number>, such as qid:3.  For
// our purposes, the actual prefix string is ignored.  The only requirement
// is that it begins with a letter in [a-zA-Z].
//
// The comment string is optional.  If it is needed, use a # character
// to begin the comment.  The remainder of the string is then treated
// as the comment.
//
// Note that features must be sorted in ascending order, by feature id.
// Also, feature id 0 is reserved for the bias term.

#ifndef SF_SPARSE_VECTOR_H__
#define SF_SPARSE_VECTOR_H__

#include <float.h>
#include <string>
#include <vector>

#define SF_UNDEFINED_VAL FLT_MAX

using std::string;
using std::vector;

// Each element of the SfSparseVector is represented as a FeatureValuePair.
// Bundling these as a struct improves memory locality.
struct FeatureValuePair {
  uint32_t id_;
  float value_;
};

class SfSparseVector {
 public:
  // Construct a new vector from a string.  Input format is svm-light format:
  // <label> <feature>:<value> ... <feature:value> # comment<\n>
  // No bias term is used.
  SfSparseVector(const vector<FeatureValuePair> &feature_vector);
  SfSparseVector(string doc_id, const vector<FeatureValuePair> &feature_vector);

  float GetSquaredNorm() const { return squared_norm_; }

  // Methods for interacting with features
  inline int NumFeatures() const { return features_.size(); }
  inline int FeatureAt(int i) const { return features_[i].id_; }
  inline float ValueAt(int i) const { return features_[i].value_; }

  // Adds a new (id, value) FeatureValuePair to the end of the vector, and
  // updates the internal squared_norm_ member.
  void PushPair (uint32_t id, float value);

  string doc_id;

  // Typically, only non-zero valued features are stored.  This vector is assumed
  // to hold feature id, feature value pairs in order sorted by feature id.  The
  // special feature id 0 is always set to 1, encoding bias.
  vector<FeatureValuePair> features_;

 private:

    // Sets up the bias term, indexed by feature id 0.
  void SetBias() { PushPair(0, 1); }

    // Exits if the input format of the file is incorrect.
  void DieFormat(const string& reason);

  // Members.
  float squared_norm_ = 0.0;
};

#endif // SF_SPARSE_VECTOR_H__
