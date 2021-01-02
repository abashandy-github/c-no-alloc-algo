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

#include <stddef.h>  /* NULL */
#include <string.h>

#include "binary_heap_with_pointers.h"


/*
 * Geenric Compare function to handle both min and max heap
 * For max heap, we just swap the nodes when calling the user-provided compare functionx
 */
static inline int
binary_heap_compare(binary_heap_t *heap,
                    binary_heap_node_t *node1,
                    binary_heap_node_t *node2)
{
  return (heap->heap_type == BINARY_HEAP_MIN ? 
	  (heap->compare_func(node1, node2)) :
          (heap->compare_func(node2, node1)));
}  

/* 
 * Intialize a heap pointer to be an empty heap
 */
binary_heap_err_t
binary_heap_init(binary_heap_t *heap,
                 binary_heap_type_t heap_type,
                 binary_heap_compare_func compare_func)
{
  if (!heap || !compare_func) {
    return (BINARY_HEAP_ERR_INVAL);
  }
  memset(heap, 0, sizeof(*heap));
  heap->compare_func = compare_func;
  heap->heap_type = heap_type;
  return (BINARY_HEAP_ERR_OK);
}

binary_heap_node_t* binary_heap_top(binary_heap_t *heap) {
  return (heap ? heap->min : NULL);
}

/* Swap parent with child. Child moves closer to the root, parent moves away. */
static inline void
binary_heap_node_swap(binary_heap_t * heap,
                      binary_heap_node_t * parent,
                      binary_heap_node_t * child) {
  binary_heap_node_t* sibling;
  binary_heap_node_t t;

  t = *parent;
  *parent = *child;
  *child = t;

  parent->parent = child;
  if (child->left == child) {
    child->left = parent;
    sibling = child->right;
  } else {
    child->right = parent;
    sibling = child->left;
  }
  if (sibling != NULL)
    sibling->parent = child;

  if (parent->left != NULL)
    parent->left->parent = parent;
  if (parent->right != NULL)
    parent->right->parent = parent;

  if (child->parent == NULL)
    heap->min = child;
  else if (child->parent->left == parent)
    child->parent->left = child;
  else
    child->parent->right = child;
}

binary_heap_err_t
binary_heap_insert(binary_heap_t *heap,
                   binary_heap_node_t* newnode) {
  binary_heap_node_t** parent;
  binary_heap_node_t** child;
  unsigned int path;
  unsigned int n;
  unsigned int k;

  /*
   * Check that the node pointers are NULL
   * If the pointers are NOT NULL, we assume that the node is either
   * inserted or corrupted
   * see comments in the header file on top of this function
   * This can help us detect the case where user attempts to remove
   * a node that is not inserted or attempts to insert a node that is
   * already inserted
   */
  if (!heap ||
      !newnode ||
      newnode->left != NULL ||
      newnode->right != NULL ||
      newnode->parent != NULL) {
    return (BINARY_HEAP_ERR_INVAL);
  }

  /* Calculate the path from the root to the insertion point.  
   * Rememeber that because we use the internal function "binary_heap_compare()", which
   * acts as if the heap is always a min heap, then we can ALWAYS treat the heap as a min
   * heap so we always insert at the left-most free node of the bottom row.
   */
  path = 0;
  for (k = 0, n = 1 + heap->num_entries; n >= 2; k += 1, n /= 2)
    path = (path << 1) | (n & 1);

  /* Now traverse the heap using the path we calculated in the previous step. */
  parent = child = &heap->min;
  while (k > 0) {
    parent = child;
    if (path & 1)
      child = &(*child)->right;
    else
      child = &(*child)->left;
    path >>= 1;
    k -= 1;
  }

  /* Insert the new node. */
  newnode->parent = *parent;
  *child = newnode;
  heap->num_entries += 1;

  /* Walk up the tree and check at each node if the heap property holds.
   * It's a min heap so parent < child must be true.
   * HEnce we will swap parent and child if parent is NOT less than child
   * Remember that the internal function "binary_heap_compare()" makes both
   * min and max heaps look the same by swaping the subtraction if the this is 
   * a max heap
   */
  while (newnode->parent != NULL &&
         binary_heap_compare(heap, newnode, newnode->parent) < 0) {
    binary_heap_node_swap(heap, newnode->parent, newnode);
  }

  return (BINARY_HEAP_ERR_OK);
}

binary_heap_err_t
binary_heap_delete(binary_heap_t  *heap,
                   binary_heap_node_t* node) {
  binary_heap_node_t* smallest;
  binary_heap_node_t** max;
  binary_heap_node_t* child;
  unsigned int path;
  unsigned int k;
  unsigned int n;

  if (!heap ||
      !node) {
    return (BINARY_HEAP_ERR_INVAL);
  }

  /*
   * Heap is empty or the node is NOT in the heap
   */
  if (heap->num_entries == 0 ||
      (heap->num_entries > 1 &&
       node->left == NULL &&
       node->right == NULL &&
       node->parent == NULL)) {
      return (BINARY_HEAP_ERR_NOENT);
  }


  /* Calculate the path from the min (the root) to the max, the left-most node
   * of the bottom row.
   */
  path = 0;
  for (k = 0, n = heap->num_entries; n >= 2; k += 1, n /= 2)
    path = (path << 1) | (n & 1);

  /* Now traverse the heap using the path we calculated in the previous step. */
  max = &heap->min;
  while (k > 0) {
    if (path & 1)
      max = &(*max)->right;
    else
      max = &(*max)->left;
    path >>= 1;
    k -= 1;
  }

  heap->num_entries -= 1;

  /* Unlink the max node. */
  child = *max;
  *max = NULL;

  if (child == node) {
    /* We're removing either the max or the last node in the tree. */
    if (child == heap->min) {
      heap->min = NULL;
    }
    /* Exit after zeroing the pointers of the node to be deleted from the heap*/
    goto out;
  }

  /* Replace the node to be deleted node with the max node. */
  child->left = node->left;
  child->right = node->right;
  child->parent = node->parent;

  if (child->left != NULL) {
    child->left->parent = child;
  }

  if (child->right != NULL) {
    child->right->parent = child;
  }

  if (node->parent == NULL) {
    heap->min = child;
  } else if (node->parent->left == node) {
    node->parent->left = child;
  } else {
    node->parent->right = child;
  }

  /* Walk down the subtree and check at each node if the heap property holds.
   * It's a min heap so parent < child must be true.  If the parent is bigger,
   * swap it with the smallest child.
   * Remember that the internal function "binary_heap_compare()" makes both
   * min and max heaps look the same by swaping the subtraction if the this is 
   * a max heap
   */
  for (;;) {
    smallest = child;
    if (child->left != NULL && binary_heap_compare(heap, child->left, smallest) < 0) {
      smallest = child->left;
    }
    if (child->right != NULL && binary_heap_compare(heap, child->right, smallest) < 0) {
      smallest = child->right;
    }
    if (smallest == child) {
      break;
    }
    binary_heap_node_swap(heap, child, smallest);
  }

  /* Walk up the subtree and check that each parent is less than the node
   * this is required, because `max` node is not guaranteed to be the
   * actual maximum in tree
   * Remember that the internal function "binary_heap_compare()" makes both
   * min and max heaps look the same by swaping the subtraction if the this is 
   * a max heap
   */
  while (child->parent != NULL && binary_heap_compare(heap, child, child->parent) < 0) {
    binary_heap_node_swap(heap, child->parent, child);
  }

  /*
   * Zero the node pointers so that we know that this node is no longer
   * inserted in the binary heap
   * This can help us detect the case where user attempts to remove
   * a node that is not inserted or attempts to insert a node that is
   * already inserted
   */
 out:
  node->left = NULL;
  node->right = NULL;
  node->parent = NULL;

  return (BINARY_HEAP_ERR_OK);
}


binary_heap_err_t
binary_heap_modify(binary_heap_t *heap,
                   binary_heap_node_t* node) {
  binary_heap_node_t* smallest;

  if (!heap ||
      !node) {
    return (BINARY_HEAP_ERR_INVAL);
  }

  /*
   * Heap is empty or the node is NOT in the heap
   */
  if (heap->num_entries == 0 ||
      (heap->num_entries > 1 &&
       node->left == NULL &&
       node->right == NULL &&
       node->parent == NULL)) {
      return (BINARY_HEAP_ERR_NOENT);
  }

  /* Walk down the subtree starting from the modified node and check
     at each node if the heap property holds.
   * It's a min heap so parent < child must be true.  If the parent is bigger,
   * swap it with the smallest child.
   * NOte that if the value of the modified node has been reduced, then we will
   * just loop once and exit
   * Remember that the internal function "binary_heap_compare()" makes both
   * min and max heaps look as min heap by swaping the subtraction if the this is 
   * a max heap
   */
  for (;;) {
    smallest = node;
    if (node->left != NULL && binary_heap_compare(heap, node->left, smallest) < 0) {
      smallest = node->left;
    }
    if (node->right != NULL && binary_heap_compare(heap, node->right, smallest) < 0) {
      smallest = node->right;
    }
    if (smallest == node) {
      break;
    }
    binary_heap_node_swap(heap, node, smallest);
  }

  /* Walk up the subtree and check that each parent is less than the node that is removed
   * This is required because node that was modified may have been
   * reduced so that it is less than the parent.
   * Note that if the modified node value was increased, then the
   * check of the while loop will fail and hence we will not enter the loop
   */
  while (node->parent != NULL && binary_heap_compare(heap, node, node->parent) < 0) {
    binary_heap_node_swap(heap, node->parent, node);
  }

  return (BINARY_HEAP_ERR_OK);
}


binary_heap_node_t *
binary_heap_pop(binary_heap_t *heap)
{
  binary_heap_node_t *min;
  binary_heap_err_t err;
  if (!heap || !heap->num_entries) {
    return (NULL);
  }
  min = heap->min;
  err = binary_heap_delete(heap, heap->min); 
  if (err != BINARY_HEAP_ERR_OK) {
    return (NULL);
  }
  return (min);
}


uint32_t binary_heap_num_entries(binary_heap_t *heap)
{
  return (heap ? heap->num_entries : 0);
}
