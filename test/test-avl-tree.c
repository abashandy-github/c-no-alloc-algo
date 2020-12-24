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
disclamer below appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(s) DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
AUTHOR(s) BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

This is a modification of the file with the same name at
   https://github.com/fragglet/c-algorithms.git
The main modification is to use the modified implementation of AVL tree in 
the src/ directory

 */
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>

#include "avl-tree.h"

#include "framework.h"

#define NUM_TEST_VALUES 1000


struct int_array_t {
  AVLTreeNode node;
  int value;
};

struct int_array_t test_array[NUM_TEST_VALUES];

#if 0
/* Tree print function - useful for debugging. */

static void print_tree(AVLTreeNode *node, int depth)
{
	struct int_array_t *value;
	int i;

	if (node == NULL) {
		return;
	}

	print_tree(avl_tree_node_child(node, AVL_TREE_NODE_LEFT), depth + 1);

	for (i=0; i<depth*6; ++i) {
		printf(" ");
	}

	value = avl_tree_node_key(node);
	printf("%i\n", value->value);

	print_tree(avl_tree_node_child(node, AVL_TREE_NODE_RIGHT), depth + 1);
}
#endif



/* Internal free function to allow calling avl_tree_remove 
 * It does NOT DO anything because our nodes are NOT allocated
 * However we MUST pass it otherwisse the avl_tree_free will NOT do anything
 */
static void internal_free(AVLTreeValue value,
                            void *context __attribute__((unused)))
{
  return;
}



/*
 * Integer comparison function
 */
int int_compare(void *key1, void *key2)
{
  return (*(int *)key1 - *(int *)key2);
}



int find_subtree_height(AVLTreeNode *node)
{
	AVLTreeNode *left_subtree;
	AVLTreeNode *right_subtree;
	int left_height, right_height;

	if (node == NULL) {
		return 0;
	}

	left_subtree = avl_tree_node_child(node, AVL_TREE_NODE_LEFT);
	right_subtree = avl_tree_node_child(node, AVL_TREE_NODE_RIGHT);
	left_height = find_subtree_height(left_subtree);
	right_height = find_subtree_height(right_subtree);

	if (left_height > right_height) {
		return left_height + 1;
	} else {
		return right_height + 1;
	}
}

/* Validates a subtree, returning its height */

int counter;

int validate_subtree(AVLTreeNode *node)
{
	AVLTreeNode *left_node, *right_node;
	int left_height, right_height;
	int *key;

	if (node == NULL) {
		return 0;
	}

	left_node = avl_tree_node_child(node, AVL_TREE_NODE_LEFT);
	right_node = avl_tree_node_child(node, AVL_TREE_NODE_RIGHT);

	/* Check the parent references of the children */

	if (left_node != NULL) {
		assert(avl_tree_node_parent(left_node) == node);
	}
	if (right_node != NULL) {
		assert(avl_tree_node_parent(right_node) == node);
	}

	/* Recursively validate the left and right subtrees,
	 * obtaining the height at the same time. */

	left_height = validate_subtree(left_node);

	/* Check that the keys are in the correct order */

	key = (int *) avl_tree_node_key(node);

	assert(*key > counter);
	counter = *key;

	right_height = validate_subtree(right_node);

	/* Check that the returned height value matches the
	 * result of avl_tree_subtree_height(). */

	assert(avl_tree_subtree_height(left_node) == left_height);
	assert(avl_tree_subtree_height(right_node) == right_height);

	/* Check this node is balanced */

	assert(left_height - right_height < 2 &&
	       right_height - left_height < 2);

	/* Calculate the height of this node */

	if (left_height > right_height) {
		return left_height + 1;
	} else {
		return right_height + 1;
	}
}

void validate_tree(AVLTree *tree)
{
	AVLTreeNode *root_node;
	int height;

	root_node = avl_tree_root_node(tree);

	if (root_node != NULL) {
		height = find_subtree_height(root_node);
		assert(avl_tree_subtree_height(root_node) == height);
	}

	counter = -1;
	validate_subtree(root_node);
}

AVLTree *create_tree(AVLTree *tree_struct)
{
  AVLTree  *tree;
  int i;

  /* Make sure to zero out the test array because we use it multiple times */
  memset(test_array, 0, sizeof(test_array));
  
  /* Create a tree and fill with nodes */
   
  tree = avl_tree_new(tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);
 
  for (i=0; i<NUM_TEST_VALUES; ++i) {
    test_array[i].value = i;
    assert(avl_tree_insert(tree, &test_array[i]) != NULL);
  }
   
  return tree;
}

void test_avl_tree_new(void)
{
  AVLTree tree_struct;
  AVLTree *tree;

  printf(":  '%s'", __FUNCTION__);
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);

  assert(tree != NULL);
  assert(avl_tree_root_node(tree) == NULL);
  assert(avl_tree_num_entries(tree) == 0);  
}

void test_avl_tree_insert_lookup(void)
{
  AVLTree *tree;
  AVLTree tree_struct;
  AVLTreeNode *node;
  unsigned int i;
  struct int_array_t *value;
  int *key;

  printf(":  '%s'", __FUNCTION__);
  
  /* Create a tree containing some values. Validate the
   * tree is consistent at all stages. */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);

  
  for (i=0; i<NUM_TEST_VALUES; ++i) {
    test_array[i].value = (int) i;
    avl_tree_insert(tree, &test_array[i]);
    
    assert(avl_tree_num_entries(tree) == i + 1);
    validate_tree(tree);
  }
  
  assert(avl_tree_root_node(tree) != NULL);
  
  /* Check that all values can be read back again */
  
  for (i=0; i<NUM_TEST_VALUES; ++i) {
    node = avl_tree_lookup_node(tree, &i);
    assert(node != NULL);
    key = avl_tree_node_key(node);
    assert(*(int *)key == (int) i);
    value = avl_tree_node_value(node);
    assert(value->value == (int) i);
  }
  
  /* Check that invalid nodes are not found */
  
  i = NUM_TEST_VALUES + 100;
  assert(avl_tree_lookup_node(tree, &i) == NULL);
  
}

void test_avl_tree_child(void)
{
  AVLTree *tree;
  AVLTree tree_struct;
  AVLTreeNode *root;
  AVLTreeNode *left;
  AVLTreeNode *right;
  struct int_array_t values[3];
  struct int_array_t *p;
  int i;

  printf(":  '%s'", __FUNCTION__);

  /* Create a tree containing some values. Validate the
   * tree is consistent at all stages. */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);
  for (i=0; i<3; ++i) {
    values[i].value = i + 1;
    avl_tree_insert(tree, &values[i]);
  }
  
  /* Check the tree */
  
  root = avl_tree_root_node(tree);
  p = avl_tree_node_value(root);
  assert(p->value == 2);
  
  left = avl_tree_node_child(root, AVL_TREE_NODE_LEFT);
  p = avl_tree_node_value(left);
  assert(p->value == 1);
  
  right = avl_tree_node_child(root, AVL_TREE_NODE_RIGHT);
  p = avl_tree_node_value(right);
  assert(p->value == 3);
  
  /* Check invalid values */
  
  assert(avl_tree_node_child(root, 10000) == NULL);
  assert(avl_tree_node_child(root, 2) == NULL);
  
}



void test_avl_tree_free(void)
{
  AVLTree *tree;
  AVLTree tree_struct;  

  printf(":  '%s'", __FUNCTION__);

  /* Try freeing an empty tree */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);
  
  avl_tree_free(tree);
  
  /* Create a big tree and free it */
  
  tree = create_tree(&tree_struct);
  avl_tree_free(tree);
}


void test_avl_tree_lookup(void)
{
  AVLTree *tree;
  int i;
  struct int_array_t *value;
  AVLTree tree_struct;
  
  printf(":  '%s'", __FUNCTION__);

  /* Create a tree and look up all values */
  
  tree = create_tree(&tree_struct);
  
  for (i=0; i<NUM_TEST_VALUES; ++i) {
    value = avl_tree_lookup(tree, &i);
    
    assert(value != NULL);
    assert(value->value == i);
  }
        
  /* Test invalid values */
  
  i = -1;
  assert(avl_tree_lookup(tree, &i) == NULL);
  i = NUM_TEST_VALUES + 1;
  assert(avl_tree_lookup(tree, &i) == NULL);
  i = 8724897;
  assert(avl_tree_lookup(tree, &i) == NULL);
  
}

void test_avl_tree_remove(void)
{
  AVLTree *tree;
  int i;
  int x, y, z;
  int value;
  unsigned int expected_entries;
  AVLTree tree_struct;

  fprintf(stdout, ":  '%s'", __FUNCTION__);

  tree = create_tree(&tree_struct);

  /* Try removing invalid entries */
  
  i = NUM_TEST_VALUES + 100;
  assert(avl_tree_remove(tree, &i) == 0);
  i = -1;
  assert(avl_tree_remove(tree, &i) == 0);
  
  /* Delete the nodes from the tree */
  
  expected_entries = NUM_TEST_VALUES;
  
  /* This looping arrangement causes nodes to be removed in a
   * randomish fashion from all over the tree. */
  
  for (x=0; x<10; ++x) {
    for (y=0; y<10; ++y) {
      for (z=0; z<10; ++z) {
        value = z * 100 + (9 - y) * 10 + x;
        assert(avl_tree_remove(tree, &value) != 0);
        validate_tree(tree);
        expected_entries -= 1;
        assert(avl_tree_num_entries(tree)
               == expected_entries);
      }
    }
  }

  /* All entries removed, should be empty now */
  
  assert(avl_tree_root_node(tree) == NULL);
  
}

#if 0
void test_avl_tree_to_array(void)
{
  AVLTree *tree, tree_struct;
  int entries[] = { 89, 23, 42, 4, 16, 15, 8, 99, 50, 30 };
  int sorted[]  = { 4, 8, 15, 16, 23, 30, 42, 50, 89, 99 };
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  unsigned int i;
  struct int_array_t **array;
  struct int_array_t  *values = malloc(num_entries * sizeof(*values));

  printf(":  '%s'", __FUNCTION__);

  /* Create tree then Add all entries to the tree */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);

  for (i=0; i<num_entries; ++i) {
    values[i].value = entries[i];
    avl_tree_insert(tree, &values[i]);
  }

  assert(avl_tree_num_entries(tree) == num_entries);

  /* Allocate the array because the APIs no logner allocate any memory*/
  array = malloc(sizeof(AVLTreeValue) * tree->num_nodes);

  
  /* Convert to an array and check the contents */

  array = (int **) avl_tree_to_array(tree, (void **)array);

  for (i=0; i<num_entries; ++i) {
    assert(array[i]->value == sorted[i]);
  }

  free(array);
  free(values);

  /* NOT Test out of memory scenario because we do NOT allocate memory*/

  /* alloc_test_set_limit(0);

	array = (int **) avl_tree_to_array(tree);
	assert(array == NULL);
	validate_tree(tree); */

	/* avl_tree_free(tree); */
}
#endif

void test_avl_tree_successor_predecessor_min_greater_or_equal_max_equal_or_less(void)
{
  AVLTree *tree, tree_struct;
  int entries[] = { 89, 23, 42, 4, 16, 15, 8, 99, 50, 30 };
  /* int sorted[]  = { 4, 8, 15, 16, 23, 30, 42, 50, 89, 99 };*/
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  unsigned int i;
  struct int_array_t *value;
  struct int_array_t  *values = malloc(num_entries * sizeof(*values));
  
  printf(":  '%s'", __FUNCTION__);

  /* Create tree then Add all entries to the tree */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);
  for (i=0; i<num_entries; ++i) {
    values[i].value = entries[i];
    avl_tree_insert(tree, &values[i]);
  }
  assert(avl_tree_num_entries(tree) == num_entries);

  /***************** Predeccessor testing **********************************/
  /* Test Predeccessor using a value that does NOT exist
    * Success means it is goind to return */
  i = 24; /* sucessor should be 23 */
  value = avl_tree_predeccessor(tree, &i);
  assert(value->value == 23);

  /* Test Predeccessor using a value that exists
   * Sucess means I should get the previous value in the tree */
  i = 23; /* sucessor should be 16 */
  value = avl_tree_predeccessor(tree, &i);
  assert(value->value == 16);

  /* Test Predeccessor with a value that is Less than the smallest value 
   * Success means we get back NULL */
  i = 3; /* Predecessor should be NULL because 4 is the smallest  */
  value = avl_tree_predeccessor(tree, &i);
  assert(value == NULL);

  /* Test Predeccessor with a value that is EQUAL to the smallest value 
   * Success means we get back NULL */
  i = 4; /* Prdeceessor should be NULL because 4 is the smallest */
  value = avl_tree_predeccessor(tree, &i);
  assert(value == NULL);
  


  /***************** testing Max that is less than or equal to **************/
  /* Test Max_Equal_Or_Less using a value that does NOT exist
    * Success means it is goind to return */
  i = 24; /* MAx that is less than ir equal to should be 23 */
  value = avl_tree_max_equal_or_less(tree, &i);
  assert(value->value == 23);

  /* Test Max_Equal_Or_Less using a value that exists
   * Success means I should get the previous value in the tree */
  i = 23; /* max that is less than or equal  should be 23 because 23 exists */
  value = avl_tree_max_equal_or_less(tree, &i);
  assert(value->value == 23);

  /* Test Max_Equal_Or_Less with a value that is Less than the smallest value 
   * Success means we get back NULL */
  i = 3; /* Max_Equal_Or_Less should be NULL because 4 is the smallest  */
  value = avl_tree_max_equal_or_less(tree, &i);
  assert(value == NULL);

  /* Test Max_Equal_Or_Less with a value that is EQUAL to the smallest value 
   * Success means we get back NULL */
  i = 4; /* Max less than or equal should be 4 because 4 is the smallest */
  value = avl_tree_max_equal_or_less(tree, &i);
  assert(value->value == 4);
  


  /***************** Successor testing **********************************/
  /* Test Successor using a value that does NOT exist
    * Success means it is goind to return */
  i = 24; /* sucessor should be 30 */
  value = avl_tree_successor(tree, &i);
  assert(value->value == 30);

  /* Test Successor using a value that exists
   * Sucess means I should get the next value in the tree */
  i = 30; /* sucessor should be 42 */
  value = avl_tree_successor(tree, &i);
  assert(value->value == 42);

  /* Test Successor with a value that is GREATER than the largets value 
   * Success means we get back NULL */
  i = 100; /* sucessor should be NULL because 99 is the largest  */
  value = avl_tree_successor(tree, &i);
  assert(value == NULL);

  /* Test Successor with a value that is EQUAL to the largest value 
   * Success means we get back NULL */
  i = 99; /* sucessor should be NULL because 99 is the largest  */
  value = avl_tree_successor(tree, &i);
  assert(value == NULL);
  


  /***************** Min that is greater than or euqal to ***************/
  /* Test Min that is greater than or equal to using a value that does NOT exist
    * Success means it is goind to return */
  i = 24; /* Min that is greater than or equal to should be 30 */
  value = avl_tree_min_equal_or_greater(tree, &i);
  assert(value->value == 30);

  /* Test Min that is greater than or equal using a value that exists
   * Success means I should get the SAME value in the tree */
  i = 30; /* min that is greater than or equal to should be 30 */
  value = avl_tree_min_equal_or_greater(tree, &i);
  assert(value->value == 30);

  /* Test Min that is greater than or equal with a value that is
   *  GREATER than the largets value
   * Success means we get back NULL */
  i = 100; /* Min greater than or equal to should be NULL because 99
              is the largest */
  value = avl_tree_min_equal_or_greater(tree, &i);
  assert(value == NULL);

  /* Test Min grreater than or equal to with a value that is EQUAL to
   *  the largest value
   * Success means we get back the largetst value */
  i = 99; /* sucessor should be 99 because 99 is the largest  */
  value = avl_tree_min_equal_or_greater(tree, &i);
  assert(value->value == 99);
  

  free(values);
  
}



/******************************** test max and min  **************************/
void test_avl_tree_min_max(void)
{
  AVLTree *tree, tree_struct;
  int entries[] = { 89, 23, 42, 4, 16, 15, 8, 99, 50, 30 };
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  int i;
  struct int_array_t *value;
  struct int_array_t  *values = malloc(num_entries * sizeof(*values));


  printf(":  '%s'", __FUNCTION__);

  /* Create tree then Add all entries to the tree */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);
  for (i=0; i<num_entries; ++i) {
    values[i].value = entries[i];
    avl_tree_insert(tree, &values[i]);
  }
  assert(avl_tree_num_entries(tree) == num_entries);

  value = avl_tree_max(tree);
  assert(value->value == 99);
  value = avl_tree_min(tree);
  assert(value->value == 4);
  
  free(values);
}


/******************************** test tree walk ******************************/


bool test_avl_tree_ascend_walk_func(AVLTreeValue value, void *context)
{
  static uint32_t i;
  int *array = (int *)context;
  int num = ((struct int_array_t *)value)->value;
  array[i] = num;
  i++;
  return (false);
}
bool test_avl_tree_descend_walk_func(AVLTreeValue value, void *context)
{
  static uint32_t i;
  int *array = (int *)context;
  int num = ((struct int_array_t *)value)->value;
  array[i] = num;
  i++;
  return (false);
}
bool test_avl_tree_abort_walk_func(AVLTreeValue value, void *context)
{
  static uint32_t i;
  int *array = (int *)context;
  int num = ((struct int_array_t *)value)->value;
  array[i] = num;
  i++;
  if (i == 3) {
    return (true);
  }
  return (false);
}
void test_avl_tree_walk(void)
{
  AVLTree *tree, tree_struct;
  int entries[] = { 89, 23, 42, 4, 16, 15, 8, 99, 50, 30 };
  int ascend[]  = { 4, 8, 15, 16, 23, 30, 42, 50, 89, 99 };
  int descend[] = {99, 89, 50, 42, 30, 23, 16, 15, 8, 4};
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  unsigned int i;
  int *array;
  struct int_array_t  *values = malloc(num_entries * sizeof(*values));

  printf(":  '%s'", __FUNCTION__);

  array  = malloc(sizeof(entries));
  /* Create tree then Add all entries to the tree */
  tree = avl_tree_new(&tree_struct,
                      offsetof(struct int_array_t, value), /* Key is right after node field */
                      (AVLTreeCompareFunc) int_compare,
                      internal_free,
                      NULL);
  for (i=0; i<num_entries; ++i) {
    values[i].value = entries[i];
    avl_tree_insert(tree, &values[i]);
  }
  assert(avl_tree_num_entries(tree) == num_entries);

  /*
   * Test ascending walk
   */
  memset(array, 0, sizeof(entries));
  avl_tree_walk(tree, false, test_avl_tree_ascend_walk_func, array);
  assert(!memcmp(array, ascend, sizeof(entries)));
  
  /*
   * Test descending walk
   */
  memset(array, 0, sizeof(entries));
  avl_tree_walk(tree, true, test_avl_tree_descend_walk_func, array);
  assert(!memcmp(array, descend, sizeof(entries)));

  /*
   * Test aborting walk
   */
  memset(array, 0, sizeof(entries));
  avl_tree_walk(tree, false, test_avl_tree_abort_walk_func, array);
  for (i = 3; i < num_entries; i++) {
    assert(array[i] != 0);
  }
  free(array);
}

static UnitTestFunction tests[] = {
	test_avl_tree_new,
	test_avl_tree_free,
	test_avl_tree_child,
	test_avl_tree_insert_lookup,
	test_avl_tree_lookup,
	test_avl_tree_remove,
	/*test_avl_tree_to_array,*/
	/*test_out_of_memory,*/
        test_avl_tree_successor_predecessor_min_greater_or_equal_max_equal_or_less,
        test_avl_tree_min_max,
        test_avl_tree_walk,
	NULL
};

int main(int argc, char *argv[])
{
	run_tests(tests);
        printf("\n");
	return 0;
}


