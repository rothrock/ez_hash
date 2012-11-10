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

#include <stdint.h>
#include "fnv.h"

typedef struct ez_hash_rec_struct {
// This datatype holds a key, corresponding value, and a pointer to the
// next key-val pair sloshing together in the same bucket.
  char* key;                          // string that we hash on.
  char* value;                        // payload of the record.
  struct ez_hash_rec_struct* next;    // points to next record in the same bucket.
} ez_hash_rec;


typedef struct ez_hash_table_struct {
// the hash data structure. 
  int hash_bits;              // a number from 0 to 31. 2**hash_bits = number of hash buckets.
  int n_members;              // number of records in the hash.
  ez_hash_rec** buckets;      // Dynamically sized at initialization, Each element of the array corresponds to a hash bucket. The
                              // subscript of the array is the hash code. Each bucket contains a list of records (ez_hash_rec).
} ez_hash_table;


ez_hash_table* ez_hash_init(uint32_t n);

uint32_t ez_hash_free(ez_hash_table* h);

uint32_t ez_hash_set(ez_hash_table* h, char* key, char* value);

char* ez_hash_get(ez_hash_table* h, char* key);

int ez_hash_del(ez_hash_table* h, char* key);

// uint32_t _get_hash_val(uint32_t bits, char* key)
//
// This is a private wrapper function around the FNV hash function.
//
// arguments:
// 
// bits - An integer between 0 and 31 used to create bitmask. The function uses
// the bitmask to trim the size of the hashcode. At the time of this writing,
// the bits shifting behaves as bits modulo 32. Ex. bits=40 actually shifts 8.
//
// key - string that we put into the FNV function.
//
// returns:
//
// The shifted hashcode.
uint32_t _get_hash_val(uint32_t bits, char* key);


ez_hash_rec* init_new_rec(char* key, char* value);
 
