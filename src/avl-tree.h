/*

Copyright (c) 2005-2008, Simon Howard

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice appear
in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

Copyright (c) 2019 Ahmed Bashandy

Permission to use, copy, modify, and/or distribute this software
for any purpose with or without fee is hereby granted, provided
that the above copyright notice and this permission notice and the 
* disclamer below appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(s) DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR(s) BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

See comments in "avl-tree.c" after the disclamer
 */

/** @file avl-tree.h
 *
 * @brief Balanced binary tree
 *
 * The AVL tree structure is a balanced binary tree which stores
 * a collection of nodes (see @ref AVLTreeNode).  Each node has
 * a key and a value associated with it.  The nodes are sorted
 * within the tree based on the order of their keys. Modifications
 * to the tree are constructed such that the tree remains
 * balanced at all times (there are always roughly equal numbers
 * of nodes on either side of the tree).
 *
 * Balanced binary trees have several uses.  They can be used
 * as a mapping (searching for a value based on its key), or
 * as a set of keys which is always ordered.
 *
 * To create a new AVL tree, use @ref avl_tree_new.  To destroy
 * an AVL tree, use @ref avl_tree_free.
 *
 * To insert a new key-value pair into an AVL tree, use
 * @ref avl_tree_insert.  To remove an entry from an
 * AVL tree, use @ref avl_tree_remove or @ref avl_tree_remove_node.
 *
 * To search an AVL tree, use @ref avl_tree_lookup or
 * @ref avl_tree_lookup_node.
 *
 * Tree nodes can be queried using the
 * @ref avl_tree_node_child,
 * @ref avl_tree_node_parent,
 * @ref avl_tree_node_key and
 * @ref avl_tree_node_value functions.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifndef ALGORITHM_AVLTREE_H
#define ALGORITHM_AVLTREE_H

#ifdef __cplusplus
extern "C" {
#endif


/**
 * A key for an @ref AVLTree.
 */
typedef void *AVLTreeKey;


/**
 * An @ref AVLTreeNode can have left and right children.
 */
typedef enum {
	AVL_TREE_NODE_LEFT = 0,
	AVL_TREE_NODE_RIGHT = 1
} AVLTreeNodeSide;

/**
 * A node in an AVL tree.
 *
 * @see avl_tree_node_left_child
 * @see avl_tree_node_right_child
 * @see avl_tree_node_parent
 * @see avl_tree_node_key
 */
typedef struct _AVLTreeNode {
  struct _AVLTreeNode *children[2];
  struct _AVLTreeNode *parent;
  int height;
} AVLTreeNode;


/**
 * Type of function used to compare keys in an AVL tree.
 *
 * @param value1   The first key.
 * @param value2   The second key.

 * @return         A negative number if node with key1 should be sorted before
 *                 node with key2, a positive number if node with key2 should
 *                 be sorted before node with key1, zero if the two keys are
 *                 equal.
 */
typedef int (*AVLTreeCompareFunc)(AVLTreeKey Key1, AVLTreeKey Key2);

/*
 * Callback function to free a node 
 * Remember that a node is now the FIRST field of the "value" structure
 */
typedef void (*AVLTreeFreeFunc)(AVLTreeNode *node, void *context);

/*
 * Callback function to walk a tree
 *
 * @param tree      The tree to walk.
 * @param context   Opaque COntext passed to avl_tree_walk() API
 *
 * @return          if the the returned value is "non-false", the walk will 
 *                  be aborted immediately
 */
typedef bool (*AVLTreeWalkFunc)(AVLTreeNode *node, void *context);
  

/**
 * An AVL tree balanced binary tree.
 * Read the top of the file "avl-tree.c" to understand more
 *
 * @see avl_tree_new
 */
typedef struct _AVLTree  {
  AVLTreeNode *root_node;
  AVLTreeCompareFunc compare_func;
  uint32_t num_nodes;
  AVLTreeFreeFunc free_func;
  intptr_t key_offset;
  void *free_context;
} AVLTree;



/**
 * Create a new AVL tree.
 *
 * @param new_tree      Pointer passed to us by the caller to be 
 *                      populated by us if we successfully create the
 *                      tree. 
 *                      It is the responsibility of the caller to allocate
 *                      and free this pointer
 * @param key_offset    Offset from the beginning of the field "node" to where the
 *                      key starts. "node" is a field in what user of this
 *                      library inserts, deletes, lookup,..., etc
 *                      NOTE:
 *                      "key_offset" determines the address that will be passed
 *                      to the compare function. How the compare function
 *                      performs the comparison or what contents to it uses to
 *                      return negative, positive, or zero is application
 *                      specific. "key_offset" just determines what will be
 *                      passed to the compare function. Hence "key_offset"
 *                      may point to anything inside or outside the value that
 *                      the user inserts/deletes/looks-up in the tree
 *                      QUESTION:
 *                      Why do we need a key? Can't we just live with "node"?
 *                      We need a key because we have to perform lookup, where
 *                      the user provides a key and the library returns a node
 *                      whose key matches the lookup key (as determined by the
 *                      compare function)
 *
 * @param compare_func  Function to use when comparing keys in the tree.
 * @param free_func;    Function used to free a node
 *                      This is optional. But it is required if the caller wants
 *                      to use the API "avl_tree_free(AVLTree *tree)"
 * @param free_context  Opaque context passed to the free_func

 * @return              The argument "new_tree": or NULL if it was not possible
 *                        to create the tree for any reason.
 */
AVLTree *avl_tree_new(AVLTree *new_tree,
                      intptr_t key_offset,
                      AVLTreeCompareFunc compare_func,
                      AVLTreeFreeFunc free_func,
                      void *free_context);


/**
 * Destroy an AVL tree.
 *
 * @param tree            The tree to destroy.
 */

void avl_tree_free(AVLTree *tree);

/**
 * Insert a new key-value pair into an AVL tree.
 *
 * @param tree            The tree.
 * @param key             The key to insert.
 * @param value           The value to insert.
 * @return                The newly created tree node containing the
 *                        key and value, or NULL if it was not possible
 *                        to insert the node because there is a already a
 *                        node with the same key
 */

AVLTreeNode *avl_tree_insert(AVLTree *tree, AVLTreeNode *new_node);

/**
 * Remove a node from a tree.
 *
 * @param tree            The tree.
 * @param node            The node to remove
 */

void avl_tree_remove_node(AVLTree *tree, AVLTreeNode *node);

/**
 * Remove an entry from a tree, specifying the key of the node to
 * remove.
 *
 * @param tree            The tree.
 * @param key             The key of the node to remove.
 * @return                Zero (false) if no node with the specified key was
 *                        found in the tree, non-zero (true) if a node with
 *                        the specified key was removed.
 */

int avl_tree_remove(AVLTree *tree, AVLTreeKey key);

/**
 * Search an AVL tree for a node with a particular key.  This uses
 * the tree as a mapping.
 *
 * @param tree            The AVL tree to search.
 * @param key             The key to search for.
 * @return                The tree node containing the given key, or NULL
 *                        if no entry with the given key is found.
 */

AVLTreeNode *avl_tree_lookup(AVLTree *tree, AVLTreeKey key);

/**
 * Find the root node of a tree.
 *
 * @param tree            The tree.
 * @return                The root node of the tree, or NULL if the tree is
 *                        empty.
 */

AVLTreeNode *avl_tree_root_node(AVLTree *tree);

/**
 * Retrieve the key for a given tree node.
 *
 * @param tree            The tree.
 * @param node            The tree node.
 * @return                The key to the given node.
 */

AVLTreeKey avl_tree_node_key(AVLTree *tree, AVLTreeNode *node);

/**
 * Find the child of a given tree node.
 *
 * @param node            The tree node.
 * @param side            Which child node to get (left or right)
 * @return                The child of the tree node, or NULL if the
 *                        node has no child on the given side.
 */

AVLTreeNode *avl_tree_node_child(AVLTreeNode *node, AVLTreeNodeSide side);

/**
 * Find the parent node of a given tree node.
 *
 * @param node            The tree node.
 * @return                The parent node of the tree node, or NULL if
 *                        this is the root node.
 */

AVLTreeNode *avl_tree_node_parent(AVLTreeNode *node);

/**
 * Find the height of a subtree.
 *
 * @param node            The root node of the subtree.
 * @return                The height of the subtree.
 */

int avl_tree_subtree_height(AVLTreeNode *node);

/**
 * Convert the keys in an AVL tree into a C array.  This allows
 * the tree to be used as an ordered set.
 *
 * @param tree   The tree.
 * @param array  Array to allocate then populate with pointer sot keys
 *
 * @return       A newly allocated C array containing all the keys
 *               in the tree, in order.  The length of the array
 *               is equal to the number of entries in the tree
 *               (see @ref avl_tree_num_entries).
 */

void  **avl_tree_to_array(AVLTree *tree, void **array);

/**
 * Retrieve the number of entries in the tree.
 *
 * @param tree            The tree.
 * @return                The number of key-value pairs stored in the tree.
 */

uint32_t avl_tree_num_entries(AVLTree *tree);



/*
 * Retrieve the successor of a key
 *  A successor of the key "x" is the node with the smallest key value that is
 *  strictly GREATER than the input key "x"
 *  I.e. if there is a node with key value "x" we do NOT want it. We want
 *       next node. If there is no next node, we will return NULL
 *
 * @param tree            The AVL tree to search.
 * @param key             The key to to find the successor of.
 * @return                The value associated with the successor, or
 *                        @ref NULL if no successor to the key was
 *                        found. 
 *                        I.e the "key" is greater than or equal to the 
 *                        node with the greatest key value
 */
AVLTreeNode *avl_tree_successor(AVLTree *tree, AVLTreeKey key);


/*
 * Retrieve the value whose key minimum key value that is greater than or 
 * equal to the passed "key" argument
 * Identical to successor except if we find a node that is equal 
 * to "key" we return it
 * @param tree            The AVL tree to search.
 * @param key             The key to to find the successor of.
 * @return                The value associated with the successor, or
 *                        @ref NULL if no successor to the key was
 *                        found. 
 *                        I.e the "key" is STRICTLY greater than the node with
 *                        the greatest key value
 */
AVLTreeNode *avl_tree_min_equal_or_greater(AVLTree *tree, AVLTreeKey key);

/**
 * A Predeccessor of the key "x" is the node with the largest key value that is
 *  strictly LESS than the input key "x"
 *  I.e. if there is anode with key value "x" we do NOT want it
 * If the function returns NULL, this means that the "key" is less than or
 * equal to the smallest key in the tree
 *
 * @param tree            The AVL tree to search.
 * @param key             The key to to find the successor of.
 * @return                The value associated with the predeccessor, or
 *                        @ref NULL if no successor to the key was
 *                        found. 
 *                        I.e the "key" is greater than or equal to the node
 *                        with the greatest key value
 */
AVLTreeNode *avl_tree_predeccessor(AVLTree *tree, AVLTreeKey key);


/**
 * Identical to predeccessor except if we find a node that is equal 
 * to "key" we return it
 *
 * @param tree            The AVL tree to search.
 * @param key             The key to to find the successor of.
 * @return                The value associated with the predeccessor, or
 *                        @ref NULL if no successor to the key was
 *                        found. 
 *                        I.e the "key" is STRICTLY less than the node with
 *                        the smallest key value
 */
AVLTreeNode *avl_tree_max_equal_or_less(AVLTree *tree, AVLTreeKey key);


/**
 * Return the value with min key or null if there are no values
 *
 * @param tree            The AVL tree to search.
 * @return                The value associated with node with min key
 *                        @ref NULL if there are no nodes
 */
AVLTreeNode *avl_tree_min(AVLTree *tree);


/**
 * Return the value with max key or null if there are no values
 *
 * @param tree            The AVL tree to search.
 * @return                The value associated with node with max key
 *                        @ref NULL if there are no nodes
 */
AVLTreeNode *avl_tree_max(AVLTree *tree);



/**  
 * Walk the tree in ascending or descending order and calls the function
 * "func" and poasses to it the "value" of a node and the "context", which 
 * is the last argument to this function
 *
 * NOTE: A user can achieve the same functionality in a loop by 
 * first getting the min (or max to walk descendingly) and then doing
 * avl_tree_successor (or avl_tree_predecessor to walk descendingly)
 *
 * @param tree           The AVL tree to search.
 * @param is_descending  If true, then the walk is done descendingly
 * @param func           the walking callback function
 *                       If the callback function returns anything
 *                       other than zero, then the call aborted
 * @param context        Opaque context passed to the callback function
 */
void avl_tree_walk(AVLTree *tree ,
                   bool is_descending,
                   AVLTreeWalkFunc func,
                   void *context);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef ALGORITHM_AVLTREE_H */

