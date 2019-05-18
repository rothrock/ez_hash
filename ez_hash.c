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
#include <unistd.h>
#include <pthread.h>

// Returns a hash code for the given string in a hash
// table created with the indicated number of bits.
Fnv32_t _get_hash_val(Fnv32_t n, char* key) {

  Fnv32_t hash, high_order_bits, bit_mask;

	if (n > 32) n = 32;

	// XOR Folding. xor the top part of the hash
	// with the bottom part of the hash.
	if (n < 32) {
		bit_mask =  (1 << n) - 1;
		hash = fnv_32_str(key, FNV1_32_INIT);
		if (n >= 16) {
			high_order_bits = hash >> n;
		} else {
			high_order_bits = hash >> (32 - n);
		}
		hash = high_order_bits ^ (hash & bit_mask);
	} else {
		hash = fnv_32_str(key, FNV1_32_INIT);
	}
  return hash;
};

// Creates a hash table with 2^n - 1 buckets.
// The hash table must be freed by the caller
// with ez_hash_free().
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

// Used inside of ez_hash_free to free records between
// the two ez_hash_rec pointers supplied in args.
void *_inner_loop_free(void* args) {

	ez_hash_rec *ptr, *next;
	ez_hash_rec** argv = (ez_hash_rec**)args;
	ez_hash_rec** start = (ez_hash_rec**)argv[0];
	ez_hash_rec** end = (ez_hash_rec **)argv[1];

	int i = 0;

	while (start <= end) {
		ptr = *start++;
		i++;
    while (ptr != NULL) {
      free(ptr->key);
      free(ptr->value);
      next = ptr->next;
      free(ptr);
      ptr = next;
    }
	}
	printf("i = %d\n", i);
	return NULL;

}

// Frees a hash table created by ez_hash_init.
// An ez_hash can have up to 2^32 buckets, so
// finding and freeing all that can take some time.
// The function uses threads to do the work concurrently.
void ez_hash_free(ez_hash_table* h) {

  Fnv32_t start, end;
	pthread_t tid[THREAD_COUNT];
	ez_hash_rec** argv[2] = {NULL, NULL};
	Fnv32_t step = h->max_bucket / THREAD_COUNT;
	Fnv32_t remainder = h->max_bucket % THREAD_COUNT;

	// If we have more threads than buckets then
	// don't bother with threading.
	if (step == 0) {
		argv[0] = &h->buckets[0];
		argv[1] = &h->buckets[h->max_bucket];
		_inner_loop_free(&argv);
		free(h->buckets);
		return;
	}

	// Free sections of the hash table concurrently
	for (int i = 0; i < THREAD_COUNT; i++) {
		start = i * step;
		end = start + step - 1;
		printf("start %u end %u\n", start, end);
		argv[0] = &h->buckets[start];
		argv[1] = &h->buckets[end];
		pthread_create(&tid[i], NULL, _inner_loop_free, &argv);
	}

	// Wait for all threads to finish.
	for (int i = 0; i < THREAD_COUNT; i++) pthread_join(tid[i], NULL);

	// Free the last bits, if any.
	if (remainder) {
		argv[0] = &h->buckets[end + 1];
		argv[1] = &h->buckets[end + remainder];
		_inner_loop_free(&argv);
	}

  free(h->buckets);
	return;
}


// Builds a record with the given key and value.
// This function is used internally in ez_hash_set.
ez_hash_rec* _init_new_rec(char* key, char* value) {

  Fnv32_t key_len = strlen(key);
  Fnv32_t val_len = strlen(value);
  ez_hash_rec* new_rec;

  if (!(new_rec = malloc(sizeof(ez_hash_rec)))) return NULL;
  if (!(new_rec->value=malloc(val_len+1))) return NULL;
  if (!(new_rec->key=malloc(key_len+1))) return NULL;
  strcpy(new_rec->key, key);
  strcpy(new_rec->value, value);
	new_rec->next = NULL;
  return new_rec;

}

// Adds a record to the given hash table. The hash table
// must exist. See ez_hash_init().
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
        if (!(ptr->value=malloc(strlen(value) + 1))) return 1;
        strcpy(ptr->value, value);
        return hval; 
      };
    }  while ( (ptr = ptr->next) );
    
    // If we are here, We didn't have a same-key collison.

    // Allocate a new record and initialize it.
    if (!(new_rec = _init_new_rec(key, value))) return 1;

    // Insert our record at the front of the list.
    new_rec->next = h->buckets[hval];
    h->buckets[hval] = new_rec;

  } else {

		// No collision. This bucket is empty.
    // Allocate a new record and drop it in here.
    if (!(new_rec = _init_new_rec(key, value))) return 1;
    new_rec->next = NULL;
    h->buckets[hval] = new_rec;
  }

	// Update our count of records.
  h->n_members++;
  
  return hval;
}

// Attempts to find the value associated with the given key.
// Returns NULL if the key isn't found.
char* ez_hash_get(ez_hash_table* h, char* key) {

  Fnv32_t hval = _get_hash_val(h->hash_bits, key);  
  ez_hash_rec* ptr = h->buckets[hval];

  if (ptr == NULL) return NULL; // key didn't hash to anything in our table.
  // Scan the list looking for a key match.
  do { if (strcmp(ptr->key, key) == 0) return (ptr->value); }  while ( (ptr = ptr->next) );
  return NULL; // No match.
}

// Attempts to delete the record associated with key.
// Returns 0 on success or 1 if the key wasn't found.
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
