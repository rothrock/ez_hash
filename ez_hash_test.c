#include <stdio.h>
#include <stdlib.h>
#include "ez_hash.h"

int usage(char* exename) {
  printf("usage: %s bits(2-32) [filename] | - \n", exename);
  return(0);
};

int main(int argc, char* argv[]) {
	int err;
	ez_hash_table *my_hash;
	char key[11], value[11];
	char *val;
	FILE* test_file = stdin;

  if (argc < 2) {usage(argv[0]); return 1;};
	if (argc > 2) test_file = fopen(argv[2], "r");


	int bits = atoi(argv[1]);
	printf("Creating with %d bits\n", bits);
	if(!(my_hash = ez_hash_init(bits))) return 1;

	while(fscanf(test_file, "%s %s\n", key, value) != EOF) {
		ez_hash_set(my_hash, key, value);
		if ((val = ez_hash_get(my_hash, key))) {
			printf("%s %s\n", key, val);
		} else {
			printf("got error looking up key %s\n", key);
			err = 2;
		}
	}

	ez_hash_free(my_hash);

	return err;
}
