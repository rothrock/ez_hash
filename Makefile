libez_hash.a: *.c *.h
	clang -g -c -Wall hash_32.c ez_hash.c
	ar rcs libez_hash.a ez_hash.o hash_32.o

example: libez_hash.a
	clang -g -o example example.c -I. -L. -lez_hash

.PHONY: install
install: libez_hash.a
	install libez_hash.a /usr/local/lib/libez_hash.a
	install ez_hash.h /usr/local/include/ez_hash.h
	install fnv.h /usr/local/include/fnv.h
	install longlong.h /usr/local/include/longlong.h

.PHONY: clean
clean:
	rm -rf example *.o *.so *.a
