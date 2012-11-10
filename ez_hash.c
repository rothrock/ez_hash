/*

Copyright (c) 2010 Joseph (Beau) Rothrock

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "fnv.h"
#include "ez_hash.h"


uint32_t _get_hash_val(uint32_t bits, char* key) {

  uint32_t hval;
  uint32_t bit_mask;

  bit_mask =  (1 << bits) - 1;
  hval = fnv_32_str(key, FNV1_32_INIT);
  hval = hval & bit_mask;
  return(hval);
};


ez_hash_table* ez_hash_init(uint32_t n) {

  ez_hash_table* a = NULL;
  uint32_t buckets = (1 << n);
  uint32_t i = 0;

  if (!(a = (ez_hash_table*)malloc(sizeof(ez_hash_table)))) return(NULL);  
  if (!(a->buckets = (ez_hash_rec**)malloc(sizeof(ez_hash_rec**)*buckets))) return(NULL);
  a->hash_bits = n;
  a->n_members = 0;
  for (i=0; i <buckets; i++) a->buckets[i] = NULL;
  return(a);
}


uint32_t ez_hash_free(ez_hash_table* h) {

  uint32_t i;
  uint32_t n = 1 << h->hash_bits;
  ez_hash_rec *ptr, *next;

  for (i = 0; i < n; i++) {
    ptr = h->buckets[i];
    while (ptr != NULL) {
      free(ptr->key);
      free(ptr->value);
      next = ptr->next;
      free(ptr);
      ptr = next;
    }
  }
  free(h->buckets);
  return(0);
}


ez_hash_rec* init_new_rec(char* key, char* value) {

  uint32_t key_len = strlen(key);
  uint32_t val_len = strlen(value);
  ez_hash_rec* new_rec;

  // allocate a new record and initialize it.
  if (!(new_rec = malloc(sizeof(ez_hash_rec)))) return(NULL);
  if (!(new_rec->value=malloc(val_len+1))) return(NULL);
  if (!(new_rec->key=malloc(key_len+1))) return(NULL);
  strcpy(new_rec->key, key);
  strcpy(new_rec->value, value);

  return(new_rec);

}


uint32_t ez_hash_set(ez_hash_table* h, char* key, char* value) {

  ez_hash_rec* new_rec;
  ez_hash_rec* ptr;
  uint32_t hval = _get_hash_val(h->hash_bits, key);

  if (h->buckets[hval]) { // We have a collision
    ptr = h->buckets[hval];
    do { // Scan the list of records to see if we have a same-key collision.
      if (strcmp(ptr->key, key) == 0) { // yes, we have a same-key collision.
        free(ptr->value);
        if (!(ptr->value=malloc(strlen(value) + 1))) return(1);
        strcpy(ptr->value, value);
        return (hval); 
      };
    }  while ( (ptr = ptr->next) );
    
    // We didn't have a same-key collison.

    // allocate a new record and initialize it.
    if (!(new_rec = init_new_rec(key, value))) return(1);

    // Insert our record at the front of the list.
    new_rec->next = h->buckets[hval];
    h->buckets[hval] = new_rec;

  } else {

    // allocate a new record and initialize it.
    if (!(new_rec = init_new_rec(key, value))) return(1);

    new_rec->next = NULL;
    h->buckets[hval] = new_rec;

  };    

  h->n_members++;
  
  return(hval);
}

char* ez_hash_get(ez_hash_table* h, char* key) {

  uint32_t hval = _get_hash_val(h->hash_bits, key);  
  ez_hash_rec* ptr = h->buckets[hval];

  if (ptr == NULL) return (NULL); // key didn't hash to anything in our table.
  // Scan the list looking for a key match.
  do { if (strcmp(ptr->key, key) == 0) return (ptr->value); }  while ( (ptr = ptr->next) );
  return(NULL); // No match.
}

int ez_hash_del(ez_hash_table* h, char* key) {

  uint32_t hval = _get_hash_val(h->hash_bits, key);
  ez_hash_rec* ptr = h->buckets[hval];
  ez_hash_rec* previous = ptr;
  
  if (ptr == NULL) return(1); // nothing in this hash bucket at all.

  // OK something may be here. 
  // Scan the list looking for a key that matches.
  while(ptr != NULL){
    if (strcmp(ptr->key, key) == 0) { // We have a match.
      if (ptr == previous) { // We matched the first record in the bucket.
        h->buckets[hval] = (h->buckets[hval])->next;
      } else { // We matched a subsequent record.
        previous->next = ptr->next; // take ptr out of the list
      }
      free(ptr->key);
      free(ptr->value);
      free(ptr);
      return(0);
    }
    previous = ptr;
    ptr = ptr->next;
  }
  return(1); // nothing matched when we scanned the list.
}
