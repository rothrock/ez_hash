#include <stdio.h>
#include <stdlib.h>
#include "ez_hash.h"

int usage(char* exename) {
  printf("usage: %s bits(2-32) key value [key value]...\n", exename);
  return(0);
};

int main(int argc, char *argv[]) {

  ez_hash_table *my_hash;
  char *key, *value;
  int bits;

  if (argc < 4) {usage(argv[0]); return 1;};
  if ((argc % 2) !=0) {usage(argv[0]); return 1;};

  bits = atoi(argv[1]);
	if ((bits < 2) || (bits > 32)) {usage(argv[0]); return 1;};

  if(!(my_hash = ez_hash_init(bits))) return 1;
	printf("Hash table inited with %d bits\n\n", bits);

	for (int i = 2; i <= (argc - 1); i += 2) {
		key = argv[i];
		value = argv[i + 1];
		printf("setting %s %s. retval is %u\n", key, value, ez_hash_set(my_hash, key, value));
		printf("Looking up %s returns %s\n", key, ez_hash_get(my_hash, key));
		printf("Deleting %s returns %d\n", key, ez_hash_del(my_hash, key));
		printf("Deleting %s again returns %d\n\n", key, ez_hash_del(my_hash, key));
	}

	printf("Freeing the hash table\n");
  ez_hash_free(my_hash);

  return 0;
}

