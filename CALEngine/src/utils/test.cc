#include <iostream>
#include <chrono>
#include <cmath>
#include <cassert>
#include "feature_parser.h"
#include "feature_writer.h"

using namespace std;
int main(int argc, char *argv[]){
    vector<SfSparseVector*> spvs_orig;
    {
        cerr<<"Reading "<<argv[1]<<endl;
        auto start = std::chrono::steady_clock::now();
        spvs_orig = CAL::utils::SVMlightFeatureParser(argv[1]).get_all();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Read "<<spvs_orig.size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    {
        cerr<<"Writing "<<argv[2]<<endl;
        auto start = std::chrono::steady_clock::now();
        auto writer = CAL::utils::BinFeatureWriter(argv[2]);
        for(auto spv: spvs_orig)
            writer.write(spv);
        writer.finish();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Wrote "<<spvs_orig.size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    {
        cerr<<"Reading "<<argv[2]<<endl;
        auto start = std::chrono::steady_clock::now();
        auto reader = CAL::utils::BinFeatureParser(argv[2]);

        vector<SfSparseVector*> spvs_bin = reader.get_all();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Wrote "<<spvs_orig.size()<<" docs in "<<duration.count()<<"ms"<<endl;

        cerr<<"Testing "<<argv[2]<<endl;
        for(int i = 0;i< spvs_orig.size(); i++){
            assert(spvs_orig[i]->features_.size() == spvs_bin[i]->features_.size());
            for(int j = 0;j < spvs_orig[i]->features_.size(); j++){
                assert(spvs_orig[i]->features_[j].id_ == spvs_bin[i]->features_[j].id_);
                assert(abs(spvs_orig[i]->features_[j].value_ - spvs_bin[i]->features_[j].value_) < 1e-6);
            }
        }
        for(auto x: spvs_bin)
            delete x;
    }

    {
        cerr<<"Writing "<<argv[3]<<endl;
        auto start = std::chrono::steady_clock::now();
        {
            auto writer = CAL::utils::SVMlightFeatureWriter(argv[3]);
            for(auto spv: spvs_orig)
                writer.write(spv);
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Wrote "<<spvs_orig.size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    {
        cerr<<"Reading "<<argv[3]<<endl;
        auto start = std::chrono::steady_clock::now();
        auto reader = CAL::utils::SVMlightFeatureParser(argv[3]);

        vector<SfSparseVector*> spvs_bin = reader.get_all();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Wrote "<<spvs_orig.size()<<" docs in "<<duration.count()<<"ms"<<endl;

        cerr<<"Testing "<<argv[3]<<endl;
        for(int i = 0;i< spvs_orig.size(); i++){
            assert(spvs_orig[i]->features_.size() == spvs_bin[i]->features_.size());
            for(int j = 0;j < spvs_orig[i]->features_.size(); j++){
                assert(spvs_orig[i]->features_[j].id_ == spvs_bin[i]->features_[j].id_);
                assert(abs(spvs_orig[i]->features_[j].value_ - spvs_bin[i]->features_[j].value_) < 1e-6);
            }
        }
        for(auto x: spvs_bin)
            delete x;
    }
}
