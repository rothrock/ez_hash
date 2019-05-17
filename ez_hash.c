/*

Copyright (c) 2019 Joseph Rothrock

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


Fnv32_t _get_hash_val(Fnv32_t bits, char* key) {

  Fnv32_t hash, high_order_bits, bit_mask;

	if (bits > 32) bits = 32;

	// XOR Folding. xor the top part of the hash
	// with the bottom part of the hash.
	if (bits < 32) {
		bit_mask =  (1 << bits) - 1;
		hash = fnv_32_str(key, FNV1_32_INIT);
		if (bits >= 16) {
			high_order_bits = hash >> bits;
		} else {
			high_order_bits = hash >> (32 - bits);
		}
		hash = high_order_bits ^ (hash & bit_mask);
	} else {
		hash = fnv_32_str(key, FNV1_32_INIT);
	}
  return hash;
};


ez_hash_table* ez_hash_init(Fnv32_t n) {

	Fnv32_t max_bucket;
	if ((n < 2) || (n > 32)) return NULL;
  ez_hash_table* a = NULL;
	if (n < 32) {
		max_bucket = (1 << n) - 1;
	} else {
		max_bucket = UINT32_MAX;
	}

  if (!(a = (ez_hash_table*)malloc(sizeof(ez_hash_table)))) return NULL;  
  if (!(a->buckets = (ez_hash_rec**)calloc((size_t)max_bucket + 1, sizeof(ez_hash_rec*)))) return NULL;
  a->hash_bits = n;
  a->n_members = 0;
	a->max_bucket = max_bucket;
  return a;
}


void ez_hash_free(ez_hash_table* h) {

  ez_hash_rec *ptr, *next, **csr, **end;
  
	csr = h->buckets;
	end = &h->buckets[h->max_bucket];

	while (csr <= end) {
		ptr = *csr++;
    while (ptr != NULL) {
      free(ptr->key);
      free(ptr->value);
      next = ptr->next;
      free(ptr);
      ptr = next;
    }
	}

  free(h->buckets);
}


ez_hash_rec* init_new_rec(char* key, char* value) {

  Fnv32_t key_len = strlen(key);
  Fnv32_t val_len = strlen(value);
  ez_hash_rec* new_rec;

  if (!(new_rec = malloc(sizeof(ez_hash_rec)))) return(NULL);
  if (!(new_rec->value=malloc(val_len+1))) return(NULL);
  if (!(new_rec->key=malloc(key_len+1))) return(NULL);
  strcpy(new_rec->key, key);
  strcpy(new_rec->value, value);
	new_rec->next = NULL;
  return(new_rec);

}


Fnv32_t ez_hash_set(ez_hash_table* h, char* key, char* value) {

  ez_hash_rec* new_rec;
  ez_hash_rec* ptr;
  Fnv32_t hval = _get_hash_val(h->hash_bits, key);

  if (h->buckets[hval]) { // We have a collision
    ptr = h->buckets[hval];
		// Scan the list of records to see if we have a same-key collision.
    do { 
			// Check for same-key collision. If so, overwrite current value
			// with the new value and leave the function.
      if (strcmp(ptr->key, key) == 0) {
        free(ptr->value);
        if (!(ptr->value=malloc(strlen(value) + 1))) return(1);
        strcpy(ptr->value, value);
        return (hval); 
      };
    }  while ( (ptr = ptr->next) );
    
    // If we are here, We didn't have a same-key collison.

    // Allocate a new record and initialize it.
    if (!(new_rec = init_new_rec(key, value))) return(1);

    // Insert our record at the front of the list.
    new_rec->next = h->buckets[hval];
    h->buckets[hval] = new_rec;

  } else {

		// No collision. This bucket is empty.
    // Allocate a new record and drop it in here.
    if (!(new_rec = init_new_rec(key, value))) return 1;
    new_rec->next = NULL;
    h->buckets[hval] = new_rec;
  }

	// Update our count of records.
  h->n_members++;
  
  return(hval);
}

char* ez_hash_get(ez_hash_table* h, char* key) {

  Fnv32_t hval = _get_hash_val(h->hash_bits, key);  
  ez_hash_rec* ptr = h->buckets[hval];

  if (ptr == NULL) return (NULL); // key didn't hash to anything in our table.
  // Scan the list looking for a key match.
  do { if (strcmp(ptr->key, key) == 0) return (ptr->value); }  while ( (ptr = ptr->next) );
  return(NULL); // No match.
}

int ez_hash_del(ez_hash_table* h, char* key) {

  Fnv32_t hval = _get_hash_val(h->hash_bits, key);
  ez_hash_rec* ptr = h->buckets[hval];
  ez_hash_rec* previous = ptr;
  
  if (ptr == NULL) return 1; // nothing in this hash bucket at all.

  // OK something may be here. 
  // Scan the list looking for a key that matches.
  while (ptr != NULL) {
    if (strcmp(ptr->key, key) == 0) { // We have a match.
      if (ptr == previous) { // We matched the very first record in the bucket.
        h->buckets[hval] = (h->buckets[hval])->next;
      } else { // We matched a subsequent record in the chain.
				// Stitch over the maching record.
        previous->next = ptr->next;
      }
      free(ptr->key);
      free(ptr->value);
      free(ptr);
			h->n_members--;
      return 0;
    }
		// Advance to the next record in the chain.
    previous = ptr;
    ptr = ptr->next;
  }
  return 1; // nothing matched when we scanned the list.
}
