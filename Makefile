libez_hash.a: *.c *.h
	clang -g -c -Wall hash_32.c ez_hash.c -pthread
	ar rcs libez_hash.a ez_hash.o hash_32.o

example: libez_hash.a
	clang -o example example.c -I. -L. -lez_hash

.PHONY: debug
debug: libez_hash.a
	clang -g -o example example.c -I. -L. -lez_hash

.PHONY: install
install: libez_hash.a
	install libez_hash.a /usr/local/lib/libez_hash.a
	install ez_hash.h /usr/local/include/ez_hash.h
	install fnv.h /usr/local/include/fnv.h
	install longlong.h /usr/local/include/longlong.h

.PHONY: test
test: libez_hash.a
	clang -o ez_hash_test ez_hash_test.c -I. -L. -lez_hash
	./ez_hash_test_script.sh

.PHONY: clean
clean:
	rm -rf example ez_hash_test *.o *.so *.a *.dSYM tmp.dat
