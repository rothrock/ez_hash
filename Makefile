CFLAGS = -D_FILE_OFFSET_BITS=64 -O

default: libez_hash.so

libez_hash.so:
	gcc $(CFLAGS) -fPIC -g -c -Wall hash_32.c ez_hash.c
	gcc -shared -o libez_hash.so hash_32.o ez_hash.o -lc

ez_hash_example:
	gcc -o ez_hash_example ez_hash_example.c -L. -lez_hash


.PHONY: install
install:
	install libez_hash.so /usr/lib/libez_hash.so
	install ez_hash.h /usr/include/ez_hash.h
	install fnv.h /usr/include/fnv.h
	install longlong.h /usr/include/longlong.h


.PHONY: clean
clean:
	rm -rf ez_hash_example *.o *.so
