#include <iostream>
#include <chrono>
#include <cassert>
#include "../src/utils/feature_parser.h"
#include "../src/utils/feature_writer.h"

using namespace std;

bool verify_dataset(const Dataset &d1, const Dataset &d2){
    for(int i = 0;i< d1.size(); i++){
        assert(d1.get_sf_sparse_vector(i).features_.size() == d2.get_sf_sparse_vector(i).features_.size());
        for(int j = 0;j < d1.get_sf_sparse_vector(i).features_.size(); j++){
            auto &feature1 = d1.get_sf_sparse_vector(i).features_[j];
            auto &feature2 = d2.get_sf_sparse_vector(i).features_[j];
            assert(feature1.id_ == feature2.id_);
            assert(abs(feature1.value_ - feature2.value_) < 1e-6);
        }
    }
}
int main(int argc, char *argv[]){
    system("mkdir data_tmp/");
    string input_svm_file = "data/oldreut.svm.fil";
    string output_bin_file = "data_tmp/oldreut.bin";
    string output_svm_file = "data_tmp/oldreut.svm.fil";
    unique_ptr<Dataset> dataset_orig;
    {
        cerr<<"Reading "<<input_svm_file<<endl;
        auto start = std::chrono::steady_clock::now();
        dataset_orig = CAL::utils::SVMlightFeatureParser(input_svm_file).get_all();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Read "<<dataset_orig->size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    {
        cerr<<"Writing "<<output_bin_file<<endl;
        auto start = std::chrono::steady_clock::now();
        auto writer = CAL::utils::BinFeatureWriter(output_bin_file);
        writer.write_dataset(*dataset_orig);
        writer.finish();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Wrote "<<dataset_orig->size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    {
        cerr<<"Reading "<<output_bin_file<<endl;
        auto start = std::chrono::steady_clock::now();
        auto reader = CAL::utils::BinFeatureParser(output_bin_file);

        auto dataset_bin = reader.get_all();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Read "<<dataset_orig->size()<<" docs in "<<duration.count()<<"ms"<<endl;

        cerr<<"Testing "<<output_bin_file<<"...";
        for(int i = 0;i< dataset_orig->size(); i++){
            assert(dataset_orig->get_sf_sparse_vector(i).features_.size() == dataset_bin->get_sf_sparse_vector(i).features_.size());
            for(int j = 0;j < dataset_orig->get_sf_sparse_vector(i).features_.size(); j++){
                auto &feature1 = dataset_orig->get_sf_sparse_vector(i).features_[j];
                auto &feature2 = dataset_bin->get_sf_sparse_vector(i).features_[j];
                assert(feature1.id_ == feature2.id_);
                assert(abs(feature1.value_ - feature2.value_) < 1e-6);
            }
        }
        cerr<<"OK!"<<endl;
    }

    {
        cerr<<"Writing "<<output_svm_file<<endl;
        auto start = std::chrono::steady_clock::now();
        {
            auto writer = CAL::utils::SVMlightFeatureWriter(output_svm_file);
            writer.write_dataset(*dataset_orig);
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Wrote "<<dataset_orig->size()<<" docs in "<<duration.count()<<"ms"<<endl;
    }

    {
        cerr<<"Reading "<<output_svm_file<<endl;
        auto start = std::chrono::steady_clock::now();
        auto reader = CAL::utils::SVMlightFeatureParser(output_svm_file);

        auto dataset_bin = reader.get_all();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds> 
            (std::chrono::steady_clock::now() - start);
        cerr<<"Read "<<dataset_orig->size()<<" docs in "<<duration.count()<<"ms"<<endl;

        cerr<<"Testing "<<output_svm_file<<endl;
        for(int i = 0;i< dataset_orig->size(); i++){
            assert(dataset_orig->get_sf_sparse_vector(i).features_.size() == dataset_bin->get_sf_sparse_vector(i).features_.size());
            for(int j = 0;j < dataset_orig->get_sf_sparse_vector(i).features_.size(); j++){
                auto &feature1 = dataset_orig->get_sf_sparse_vector(i).features_[j];
                auto &feature2 = dataset_bin->get_sf_sparse_vector(i).features_[j];
                assert(feature1.id_ == feature2.id_);
                assert(abs(feature1.value_ - feature2.value_) < 1e-6);
            }
        }
    }

    system("rm -rf ./data_tmp");
}
