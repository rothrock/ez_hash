#include <stdio.h>
#include "ez_hash.h"

int usage(char* exename) {
  printf("usage: %s bits key value\n", exename);
  return(0);
};

int main(int argc, char *argv[]) {

  ez_hash_table *my_hash;
  char *key, *value;
  int bits;

  if (argc != 4) {usage(argv[0]); return(1);};

  bits = atoi(argv[1]);
  key = argv[2];
  value = argv[3];

  my_hash = ez_hash_init(bits);

  printf("The hashcode is %u for %s %s\n", ez_hash_set(my_hash, key, value), key, value);

  printf("Looking up key %s returns %s\n", key, ez_hash_get(my_hash, key));

  printf("Deleting %s returns %d\n", key, ez_hash_del(my_hash, key));

  printf("Deleting %s again returns %d\n", key, ez_hash_del(my_hash, key));

  printf("The hashcode is %u for %s %s\n", ez_hash_set(my_hash, "inez", "schneider"), "inez", "schneider");

  printf("The hashcode is %u for %s %s\n", ez_hash_set(my_hash, "carla", "schneider"), "carla", "schneider");

  printf("The hashcode is %u for %s %s\n", ez_hash_set(my_hash, "fred", "schneider"), "fred", "schneider");

  printf("Looking up key %s returns %s\n", "fred", ez_hash_get(my_hash, "fred"));

  printf("Looking up key %s returns %s\n", "carla", ez_hash_get(my_hash, "carla"));

  printf("Deleting %s returns %d\n", "carla", ez_hash_del(my_hash, "carla"));

  printf("Deleting %s again returns %d\n", "carla", ez_hash_del(my_hash, "carla"));

  printf("Freeing the hash table returns %d\n", ez_hash_free(my_hash));

  return(0);

}

