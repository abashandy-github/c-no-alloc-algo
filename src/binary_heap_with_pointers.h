/* Copyright (c) 2013, Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * 
 * Copyright (c) 2019 Ahmed Bashandy
 * 
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice and the 
 * disclamer below appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR(s) BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * This file is based on the header file
 *   https://github.com/libuv/libuv/blob/v1.x/src/heap-inl.h
 * There are few main changes
 * - We modified the file so that there is a separate header file with
 *   declarations and another .c file with the actual code
 * - We  allow both min heap and max heap
 * - We added a function "binary_heap_modify" to allow modifying a node
 * - Functions return error codes
 */

#ifndef __BINARY_HEAP_WITH_POINTERS_H__
#define __BINARY_HEAP_WITH_POINTERS_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stddef.h>  /* NULL */

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Error codes returned by the functions
 */
typedef enum binary_heap_err_t_ {
  BINARY_HEAP_ERR_OK = 0, /* Success */
  BINARY_HEAP_ERR_INVAL, /* one or more arguments invalid */
  BINARY_HEAP_ERR_DUP, /* Entries have same value (comp_func returned zero) */
  BINARY_HEAP_ERR_NOENT, /* Entry not found or heap empty */
  BINARY_HEAP_ERR_NUM
} binary_heap_err_t;


/*
 * A single node
 */
typedef struct binary_heap_node_t_ {
  struct binary_heap_node_t_* left;
  struct binary_heap_node_t_* right;
  struct binary_heap_node_t_* parent;
} binary_heap_node_t;


/**
 * Heap type.  
 *
 * If a heap is a min heap (@ref BINARY_HEAP_MIN), the entry with the
 * lowest priority are stored at the top of the heap and will be the
 * first returned.  A entry "e" is said to be "lowest priority" or
 * "minimum" of the for every other entry "E" in the heap,
 * compare_func(e,E) returns a negative value
 * If a heap is a max heap (@ref BINARY_HEAP_MAX), the values with the
 * greatest priority are stored at the top of the heap.  A entry "e"
 * is said to be "greatest priority" or "maximum" if the for every
 * other entry "E" in the heap, "compare_func(e,E)" returns a positive
 * value
 */
typedef enum {
  /*
   * minimum heap. 
   */
  BINARY_HEAP_MIN = 0,
  /*
   * A maximum heap. 
   */
  BINARY_HEAP_MAX,

  BINARY_HEAP_NUM
} binary_heap_type_t;

 /**
 * Type of function used to compare values in a binary heap.
 *
 * @param node1  Address of the "node" field in The first entry
 * @param node2  Address of the "node" field in The second entry
 * @return    negative number if 1st entry less (lower priority) than 1st
 *            positive number if 1st entry greater (higher priority) than 2nd
 *            zero if the two are equal.
 */
typedef int (*binary_heap_compare_func)(binary_heap_node_t *node1,
                                        binary_heap_node_t *node2);


/**
 * A binary heap data structure.
 */
typedef struct _binary_heap_t_ {
  binary_heap_type_t heap_type;
  uint32_t num_entries;
  binary_heap_compare_func compare_func;
  binary_heap_node_t *min;
} binary_heap_t;

/**
 * Initialize the passed heap pointer to become an empty binary heap
 *
 * @param heap          The heap to be created. Memory MUST be provided
 *                      by yhe caller
 * @param heap_type     The type of heap: min heap or max heap.
 * @param compare_func  Pointer to a function used to compare the priority
 *                      of values in the heap.
 * @return              BINARY_HEAP_ERR_OK if success, otherwise an error code
 */
binary_heap_err_t
binary_heap_init(binary_heap_t *heap,
                binary_heap_type_t heap_type,
                binary_heap_compare_func compare_func);


/**
 * Find the number of values stored in a binary heap.
 *
 * @param heap             The heap.
 * @return                 The number of entries in the heap.
 */
uint32_t binary_heap_num_entries(binary_heap_t *heap);


/**
 * Remove the top node from a binary heap.
 *
 * @param heap The heap.
 * @return     a pointer to the node at the top of the heap or NULL if the
 *             heap is empty
 *             It will also return NULL if there is an error
 */
binary_heap_node_t *
binary_heap_pop(binary_heap_t *heap);

/**
 * Return a pointer to the top node from a binary heap WITHOUT
 * removing from the heap
 *
 * @param heap The heap.
 * @return     a pointer to the node at the top of the heap or NULL if the
 *             heap is empty
 *             It will also return NULL if there is an error
 */
binary_heap_node_t*
binary_heap_top(binary_heap_t *heap);

/**
 * Insert an entry into a binary heap.
 * If this is the first time this ode is ever inserted, then the user
 * MUST zero out the contents of "newnode" pointers because the
 * library assume that the non-zero contents means the node is already
 * inserted or is corrupted

 * @param heap     The heap to insert into.
 * @param entry    The entry to insert.
 * @return         BINARY_HEAP_ERR_OK if success, otherwise an error code
 */  
binary_heap_err_t
binary_heap_insert(binary_heap_t* heap,
                   binary_heap_node_t *newnode);

/**
 * Deletes a node from the heap
 * @param heap   The heap.
 * @param node   The node to be deleted from the heap.
 * 
 * @return BINARY_HEAP_ERR_OK if successful
 */
binary_heap_err_t
binary_heap_delete(binary_heap_t * heap,
            binary_heap_node_t * node);


/**
 * The user has modified the value of a node and the user is asking us
 * to update the heap according to the new value of the node:
 * 1. Node has increased or decreased by is still less than its parent
 *   and greater than its children ==> No change in heap
 * 2. Node value increased and became greater than one or both children
 *    ==> push the node down until the heap property is satisified
 * 3. Node value decreased and became smaller than its parent
 *    ==> bubble the node up until it is smaller than its parent or until the 
 *        node is is the min
 * @param heap   The heap.
 * @param node   The node to be deleted from the heap.
 * 
 * @return BINARY_HEAP_ERR_OK if successful
 */
binary_heap_err_t
binary_heap_modify(binary_heap_t * heap,
                   binary_heap_node_t * node);



#ifdef __cplusplus
}
#endif

#endif  /* #ifndef __BINARY_HEAP_WITH_POINTERS_H__*/
