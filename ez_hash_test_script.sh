#!/usr/local/bin/bash -ex

cat /dev/urandom | env LC_CTYPE=C tr -dc 'a-zA-Z' | \
	fold -w 9 | tr "\n" " " | fold -w 20 | \
	head -n 1000 > tmp.dat

./ez_hash_test  2 tmp.dat
./ez_hash_test 16 tmp.dat
./ez_hash_test 17 tmp.dat
./ez_hash_test 24 tmp.dat
./ez_hash_test 31 tmp.dat
./ez_hash_test 32 tmp.dat
