CC = g++ -O3 -Wall -pthread --std=c++14

cli:
	$(CC) src/bmi_cli.cc src/bmi_para.cc src/bmi.cc src/features.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-weight-vector.cc src/scorer.cc src/sofiaml/sofia-ml-methods.cc src/utils/feature_parser.cc -o bmi_cli

fcgi:
	$(CC) -lfcgi -lfcgi++ src/bmi_fcgi.cc src/bmi_para.cc src/bmi.cc src/features.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-weight-vector.cc src/scorer.cc src/sofiaml/sofia-ml-methods.cc src/utils/feature_parser.cc -o bmi_fcgi

sofia_cli:
	$(CC) src/sofia_cli.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-weight-vector.cc src/scorer.cc src/sofiaml/sofia-ml-methods.cc -o sofia_cli

clean:
	rm -f bmi_cli
	rm -f bmi_fcgi
