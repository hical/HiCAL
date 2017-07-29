CC = g++ -O3 -Wall -pthread --std=c++14

cli:
	$(CC) src/bmi_cli.cc src/bmi_para.cc src/bmi.cc src/features.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-weight-vector.cc src/scorer.cc src/sofiaml/sofia-ml-methods.cc src/utils/feature_parser.cc -o bmi_cli

fcgi:
	$(CC) -I/home/nghelani/fcgi/xxx/include src/bmi_fcgi.cc src/bmi_para.cc src/bmi.cc src/features.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-weight-vector.cc src/scorer.cc src/sofiaml/sofia-ml-methods.cc src/utils/feature_parser.cc -o bmi_fcgi -L/home/nghelani/fcgi/xxx/lib/ -lfcgi -lfcgi++

sofia_cli:
	$(CC) src/sofia_cli.cc src/sofiaml/sf-data-set.cc src/sofiaml/sf-sparse-vector.cc src/sofiaml/sf-weight-vector.cc src/scorer.cc src/sofiaml/sofia-ml-methods.cc -o sofia_cli

test:
	$(CC) src/test_scorer.cc src/scorer.cc src/sofiaml/sf-sparse-vector.cc src/utils/feature_parser.cc -o test_scorer

clean:
	rm -f bmi_cli
	rm -f bmi_fcgi
