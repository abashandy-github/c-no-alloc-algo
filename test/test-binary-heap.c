/*
 * 
 * Copyright (c) 2019 Ahmed Bashandy
 * 
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice and the 
 * disclamer below appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(s) DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR(s) BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 * This file 
 * - Tests the routines in binary_heap_with_pointers.c
 * - Prints failures in red and successes in green
 * - works with the binary heap implementation that we have here
 */

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

#include "binary_heap_with_pointers.h"

#define COLOR_NORMAL   "\x1B[0m"
#define COLOR_RED   "\x1B[31m"
#define COLOR_GREEN   "\x1B[32m"
#define COLOR_YELLOW   "\x1B[33m"
#define COLOR_BLUE   "\x1B[34m"
#define COLOR_MAGENTA   "\x1B[35m"
#define COLOR_CYAN   "\x1B[36m"
#define COLOR_WIHTE   "\x1B[37m"
#define COLOR_RESET "\x1B[0m"

#define DEFAULT_NUM_TEST_VALUES 2000
#define MAX_NUM_TEST_VALUES 10000

uint32_t num_fail;

struct int_array_st {
  binary_heap_node_t node;
  int value;
};


/********************** U T I L I T Y   F U N C T I O N S ********************/
/*
 * Comparing the values in the "int_array_st
 * NOTE:
 * In our structure, we put the node as the first field. Hence we can just
 * typecast the passed node pointer to our structre
 */
int int_compare(binary_heap_node_t *n1, binary_heap_node_t *n2)
{
  struct int_array_st *val1 = (struct int_array_st *)n1;
  struct int_array_st *val2 = (struct int_array_st *)n2;
  return (val1->value - val2->value);
}

/* Print error in read color */
__attribute__ ((format (printf, 1, 2)))
static void print_error(char *string, ...)
{
  va_list args;
  va_start(args, string);
  fprintf(stderr, COLOR_RED);
  vfprintf(stderr, string, args);
  fprintf(stderr, COLOR_RESET);
  va_end(args);
}


void
test_populate_test_array(struct int_array_st *test_array,
                         uint32_t passed_num_entries)
{
  int i;
  /* Prapare the values to be inserted */
  memset(test_array, 0, passed_num_entries*sizeof(*test_array));
  for (i = 0; i < passed_num_entries; i++) {
    test_array[i].value = i+1;
  }
}

uint32_t
test_populate_and_verify_heap(binary_heap_t *heap,
                              struct int_array_st *test_array,
                              bool is_populate,
                              uint32_t passed_num_entries,
                              char *test_case)
{
  int i, x, index;
  uint32_t local_fail = 0;
  binary_heap_err_t err;
  uint32_t num_entries = passed_num_entries;
  uint32_t *indeces = malloc(passed_num_entries * sizeof(*indeces));

  /* Initialize the test array */
  if (is_populate) {
    test_populate_test_array(test_array, passed_num_entries);
  }
  
  
  /* Populate indeces */
  for (i = 0; i < passed_num_entries; i++) {
    indeces[i] = i;
  }

  /* Init random to get the same sequence everytime */
  srandom(passed_num_entries + 10);
  
  /* Insert into the heap at random */
  for (i = 0; i < passed_num_entries; i++) {
    struct int_array_st *node;
    index = random() % num_entries;
    x = indeces[index];
    node = (struct int_array_st *)test_array[x].node.parent;
    err = binary_heap_insert(heap, &test_array[x].node);
    if (err != BINARY_HEAP_ERR_OK) { 
      print_error("\n%s %d: Cannot insert %dth item '%d' in %s heap %p %p %p parent = %d:%d",
                  test_case,  __LINE__,
                  x, test_array[x].value,
                  heap->heap_type == BINARY_HEAP_MIN ? "min": "max",
                  test_array[x].node.parent,
                  test_array[x].node.left,
                  test_array[x].node.right,
                  node ? node->value : -1,
                  err);
      local_fail++;
      goto out;
    }
    /* Move the last index to the location of index that was used */
    indeces[index] = indeces[num_entries - 1];
    num_entries--;
  }
  num_entries = binary_heap_num_entries(heap);
  if (num_entries != passed_num_entries) {
    print_error("\n%s %d Expecting %d items but only found %d in %s heap",
                __FUNCTION__,  __LINE__,
                passed_num_entries, num_entries,
                heap->heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
  }
 out:
  free(indeces);
  return (local_fail);
}

/**************** V E R I F I C A T I O N   F U N C T I O N *******************/

/*
 * Verify that sequential pop results in sorted array
 * I.E. The heap is correctly structured and pop works
 */
uint32_t
test_binary_heap_verify_sort(binary_heap_t *heap,
                             struct int_array_st *test_array,
                             char *test_case,
                             uint32_t num_entries)
{
  int i;
  struct int_array_st *min, *prev_min;
  int local_fail = 0;
  
  prev_min = (struct int_array_st *)binary_heap_pop(heap);
  for (i = 1; i < num_entries; i++) {
    min = (struct int_array_st *)binary_heap_pop(heap);
    if (!min) {
      print_error("\n%s %d: %dth item is NULL", test_case, __LINE__, i);
      local_fail++;
      goto out;
    }
    /* check that pointers are NULL AFTER poping the min  */
    if (min->node.parent != NULL ||
        min->node.left != NULL ||
        min->node.right != NULL) {
      print_error("\n%s %d %dth item value '%d' has non-NULL pointer %s %s %s",
                  test_case, __LINE__,  
                  i, min->value,
                  min->node.parent != NULL ? "parent" : "",
                  min->node.left != NULL ? "left" : "",
                  min->node.right != NULL ? "right" : "");
      local_fail ++;
      goto out;
    }
    if (prev_min->value > min->value &&
        heap->heap_type == BINARY_HEAP_MIN) {
      print_error("\n%s %d: %dth item value '%d' is GREATER that %dth item value '%d'",
                  test_case,  __LINE__,
                  i - 1, prev_min->value, i, min->value);
      local_fail ++;
    } else  if (prev_min->value < min->value &&
                heap->heap_type == BINARY_HEAP_MAX) {
      print_error("\n%s %d: %s %dth item value '%d' is LESS that %dth item value '%d'", __FUNCTION__, __LINE__,
                  test_case,
                  i - 1, prev_min->value, i, min->value);
      local_fail ++;
    }
    prev_min = min;
  }

  /*
   * Re-insert them again to allow for re-testing
   * We should NOT re-init the array because we deleted all entries and hence
   * re-inserted all entries again should all succeed
   */
  local_fail+= test_populate_and_verify_heap(heap,
                                             test_array,
                                             false,
                                             num_entries,
                                             test_case);

 out:
  return (local_fail);
}  


/**************************** H E A P   I N S E R T ***********************/

void test_binary_heap_insert(binary_heap_type_t heap_type,
                             uint32_t passed_num_entries)
{
  binary_heap_t heap;
  binary_heap_err_t err;
  uint32_t local_fail = 0;
  struct int_array_st *test_array =
    malloc(passed_num_entries * sizeof(*test_array));


  if (!test_array) {
    print_error("\n%s %d: Cannot alloc %zu bytes heap type %s",
                __FUNCTION__, __LINE__,
                passed_num_entries*sizeof(*test_array),
                heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }


  err = binary_heap_init(&heap,
                         heap_type, int_compare);
  if (err != BINARY_HEAP_ERR_OK) {
    print_error("\n%s %d: Cannot init heap type %s:%d",
                __FUNCTION__, __LINE__,
                heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }

  /* Let's insert the values at random */
  if ((local_fail += test_populate_and_verify_heap(&heap,
                                                   test_array,
                                                   true,
                                                   passed_num_entries,
                                                   (char *)__FUNCTION__)) != 0) {
    goto out;
  }

  /*
   * Verify that the items are sorted
   */
  local_fail += test_binary_heap_verify_sort(&heap, test_array,
                                             (char *)__FUNCTION__,
                                             passed_num_entries);


 out:
  num_fail += local_fail;

  if (num_fail) {
    print_error("\nTest %s in %s heap FAILED !!",
                __FUNCTION__, 
                heap_type == BINARY_HEAP_MIN ? "min": "max");
  } else {
    fprintf(stdout, COLOR_GREEN);
    fprintf(stdout, "\nTest '%s' in %s heap succeeded with %d items",
            __FUNCTION__, 
            heap_type == BINARY_HEAP_MIN ? "min": "max",
            passed_num_entries);
    fprintf(stdout, COLOR_RESET);
  }
  if (test_array) {
    free(test_array);
  }

}



/**************************** H E A P   D E L E T E ***********************/


void test_binary_heap_delete(binary_heap_type_t heap_type,
                             uint32_t passed_num_entries)
{
  binary_heap_t heap;
  binary_heap_err_t err;
  int i, x, index;
  uint32_t local_fail = 0;
  struct int_array_st *test_array =
        malloc(passed_num_entries * sizeof(*test_array));
  uint32_t *indeces =
    malloc(passed_num_entries * sizeof(*indeces));
  uint32_t num_entries = passed_num_entries;

  if (!test_array) {
    print_error("\n%s %d: Cannot alloc %zu bytes heap type %s",
                __FUNCTION__, __LINE__,
                passed_num_entries*sizeof(*test_array),
                heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }

  if (!indeces) {
    print_error("\n%s %d: Cannot alloc %zu bytes heap type %s",
                __FUNCTION__, __LINE__,
                passed_num_entries*sizeof(*indeces),
                heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }

  /* create the heap */
  err = binary_heap_init(&heap,
                         heap_type, int_compare);
  if (err != BINARY_HEAP_ERR_OK) {
    print_error("\n%s %d: Cannot init heap type %s:%d",
                __FUNCTION__, __LINE__,
                heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  
  /* Let's insert the values at random */
  if ((local_fail += test_populate_and_verify_heap(&heap,
                                     test_array,
                                     true,
                                     passed_num_entries,
                                     (char *)__FUNCTION__)) != 0) {
    goto out;
  }
  
  /* 
   * prepare the randmon indeces 
   */
  /* Populate indeces */
  for (i = 0; i < passed_num_entries; i++) {
    indeces[i] = i;
  }
  /* Init random to get the same sequence everytime Let's init with a
  * differnt value that what was inserted so that the deletion is not
  * the same as the insertion
  */
  srandom(passed_num_entries + 30);
  
  /*
   * Let's delete 1/2 of the values in pseudo random order that is DIFFERENT
   * from how they are inserted
   */
  for (i = 0; i < passed_num_entries/2; ++i) {
    index = random() % num_entries;
    x = indeces[index];
    err = binary_heap_delete(&heap, &test_array[x].node);
    if (err != BINARY_HEAP_ERR_OK) { 
      print_error("\n%s %d: Cannot DELETE %dth item at %d with index %d '%d' "
                  "from %s heap num_ENTRIES %d %p %p %p:%d",
                  __FUNCTION__, __LINE__,
                  i, index, x, test_array[x].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max",
                  heap.num_entries,
                  test_array[x].node.left, test_array[x].node.right,
                  test_array[x].node.parent,
                  err);
      local_fail++;
      goto out;
    }
    /* Verify that all pointers are NULL after deleteion */
    if (test_array[x].node.parent != NULL ||
        test_array[x].node.left != NULL ||
        test_array[x].node.right != NULL) {
      print_error("\n%s %d: %dth item value '%d' has non-NULL %s %s %s",
                  __FUNCTION__,  __LINE__,
                  i, test_array[x].value,
                  test_array[x].node.parent != NULL ? "parent" : "",
                  test_array[x].node.left != NULL ? "left" : "",
                  test_array[x].node.right != NULL ? "right" : "");
      local_fail ++;
      goto out;
    }
    
    /* Move the last index to the location of index that was used */
    indeces[index] = indeces[num_entries - 1];
    num_entries--;
  }

  /* Let's verify that the remaining entries still obey the heap
     property after the deletion of 1/2 of the items 
  Remember that passed_num_entries may not always be didivsible by 2*/
  local_fail +=
    test_binary_heap_verify_sort(&heap, test_array,
                                 (char *)__FUNCTION__,
                                 passed_num_entries - passed_num_entries/2);


 out:
  num_fail += local_fail;

  if (num_fail) {
    print_error("\n****Test '%s' in %s heap FAILED !!",
                __FUNCTION__, 
                heap_type == BINARY_HEAP_MIN ? "min": "max");
  } else {
    fprintf(stdout, COLOR_GREEN);
    fprintf(stdout, "\nTest '%s' in %s heap succeeded with %d items",
            __FUNCTION__, 
            heap_type == BINARY_HEAP_MIN ? "min": "max",
            passed_num_entries);
    fprintf(stdout, COLOR_RESET);
  }
  if (test_array) {
    free(test_array);
  }
  if (indeces) {
    free(indeces);
  }
}
  


/*************** H E A P   I N S E R T   A N D   D E L E T E *****************/

void test_binary_heap_insert_delete(binary_heap_type_t heap_type,
                                    uint32_t passed_num_entries)
{
  binary_heap_t heap;
  binary_heap_err_t err;
  int i, x, index;
  uint32_t local_fail = 0;
  struct int_array_st *test_array =
        malloc(passed_num_entries * sizeof(*test_array));
  uint32_t *indeces =
    malloc(passed_num_entries * sizeof(*indeces));
  uint32_t *reinsert_indeces =
        malloc(passed_num_entries * sizeof(*reinsert_indeces));
  uint32_t num_entries;

  if (!test_array) {
    print_error("\n%s %d: Cannot alloc %zu bytes heap type %s",
                __FUNCTION__, __LINE__,
                passed_num_entries*sizeof(*test_array),
                heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }
  if (!indeces) {
    print_error("\n%s %d: Cannot alloc %zu bytes heap type %s",
                __FUNCTION__, __LINE__,
                passed_num_entries*sizeof(*indeces),
                heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }
  if (!reinsert_indeces) {
    print_error("\n%s %d: Cannot alloc %zu bytes heap type %s",
                __FUNCTION__, __LINE__,
                passed_num_entries*sizeof(*reinsert_indeces),
                heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }


  /* Initialize the test array */
  test_populate_test_array(test_array,passed_num_entries);
  

  /* create the heap */
  err = binary_heap_init(&heap,
                         heap_type, int_compare);
  if (err != BINARY_HEAP_ERR_OK) {
    print_error("\n%s %d: Cannot init heap type %s:%d",
                __FUNCTION__, __LINE__,
                heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  
  /* Let's insert the values at random */
  if ((local_fail += test_populate_and_verify_heap(&heap,
                                     test_array,
                                     false,
                                     passed_num_entries,
                                     (char *)__FUNCTION__)) != 0) {
    goto out;
  }

  /*
   * Let's delete some of the values in pseudo random order that is DIFFERENT
   * from how they are inserted
   */
  /* Populate indeces */
  for (i = 0; i < passed_num_entries; i++) {
    indeces[i] = i;
  }
  /* Init random to get the same sequence everytime */
  srandom(passed_num_entries*2);
  num_entries = passed_num_entries;
  for (i = 0; i < passed_num_entries/2; i++){
    index = random() % num_entries;
    x = indeces[index];
        
    err = binary_heap_delete(&heap, &test_array[x].node);
    if (err != BINARY_HEAP_ERR_OK) { 
      print_error("\n%s %d: Cannot DELETE %dth item '%d' from %s heap :%d",
                  __FUNCTION__,  __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
    }
    /* Verify that all pointers are NULL after deleteion */
    if (test_array[x].node.parent != NULL ||
        test_array[x].node.left != NULL ||
        test_array[x].node.right != NULL) {
      print_error("\n%s %d: %dth item index %d value '%d' has non-NULL %s %s %s",
                  __FUNCTION__,  __LINE__,
                  i, x, test_array[x].value,
                  test_array[x].node.parent != NULL ? "parent" : "",
                  test_array[x].node.left != NULL ? "left" : "",
                      test_array[x].node.right != NULL ? "right" : "");
      local_fail ++;
      goto out;
    }

    /* Store the index so that we can re-insert the values that were removed */
    reinsert_indeces[i] = x;

    /* Move the last index to the location of index that was used */
    indeces[index] = indeces[num_entries - 1];
    num_entries--;    
  }

  /*
   * Let's re-insert the same items but with different order that how
   * they where deleted
   */
  num_entries = passed_num_entries/2;
  for (i = 0; i < passed_num_entries/2; i++) {
    index = random() % num_entries;
    x = reinsert_indeces[index];
    err = binary_heap_insert(&heap, &test_array[x].node);
    if (err != BINARY_HEAP_ERR_OK) { 
      print_error("\n%s %d: Cannot RE-INSERT %dth item index %d '%d' "
                  "in %s heap :%d",
                  __FUNCTION__, __LINE__,
                  i, x, test_array[x].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
    }
    /* Verify that at least ONE pointer is non-NULL after insertion because
     * we know that the  heap already has nodes*/
    if (test_array[x].node.parent == NULL &&
        test_array[x].node.left == NULL &&
        test_array[x].node.right == NULL) {
      print_error("\n%s %d: %dth item index %d value '%d' has ALL POINTERS NULL",
                  __FUNCTION__, __LINE__,
                  i, x, test_array[x].value);
      local_fail ++;
      goto out;
    }

    /* Move the last index to the location of index that was used */
    reinsert_indeces[index] = reinsert_indeces[num_entries - 1];
    num_entries--;    
  }


  /* Let's verify that they are still sorted after deletion then
     insertion of some items*/
  local_fail += test_binary_heap_verify_sort(&heap, test_array,
                                             (char *)__FUNCTION__,
                                             passed_num_entries);


 out:
  num_fail += local_fail;

  if (num_fail) {
    print_error("\n****Test '%s' in %s heap FAILED !!",
                __FUNCTION__, 
                heap_type == BINARY_HEAP_MIN ? "min": "max");
  } else {
    fprintf(stdout, COLOR_GREEN);
    fprintf(stdout, "\nTest '%s' in %s heap succeeded with %d items",
            __FUNCTION__, 
            heap_type == BINARY_HEAP_MIN ? "min": "max",
            passed_num_entries);
    fprintf(stdout, COLOR_RESET);
  }
  if (test_array) {
    free(test_array);
  }
  if (indeces) {
    free(indeces);
  }
  if (reinsert_indeces) {
    free(reinsert_indeces);
  }
}
  


/**************************** H E A P   M O D I F Y ***********************/

void test_binary_heap_modify(binary_heap_type_t heap_type)
{
  int entries[] =        {78, 24, 39, 3, 18, 99, 7, 15, 49, 31, 103, 65, 110};
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  unsigned int i;
  struct int_array_st *test_array = malloc(num_entries * sizeof(*test_array));
  binary_heap_t heap;
  binary_heap_err_t err;
  uint32_t local_fail = 0;


  /* Initialize the test array */
  memset(test_array, 0, num_entries * sizeof(*test_array));
  for (i=0; i<num_entries; ++i) {
    test_array[i].value = entries[i];
  }


  /* create the heap */
  err = binary_heap_init(&heap,
                         heap_type, int_compare);
  if (err != BINARY_HEAP_ERR_OK) {
    print_error("\n%s %d: Cannot init heap type %s:%d",
                __FUNCTION__, __LINE__,
                heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }

  
  /* Let's insert the values. Note that they are already at random */
  for (i = 0; i < num_entries; i++) {
    err = binary_heap_insert(&heap, &test_array[i].node);
    if (err != BINARY_HEAP_ERR_OK) { 
      print_error("\n%s %d: Cannot insert %dth item '%d' in %s heap :%d",
                  __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
    }
  }



  /* Increment a node without modifying its rank among the others */
  i = 2;
  test_array[i].value += 1;
  err = binary_heap_modify(&heap, &test_array[i].node);
  if (err != BINARY_HEAP_ERR_OK) { 
    print_error("\n%s %d: Cannot INCREMENT %dth item '%d' in %s heap :%d",
                __FUNCTION__, __LINE__,
                i, test_array[i].value,
                heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  if (test_binary_heap_verify_sort(&heap, test_array,
                                   (char *)__FUNCTION__,
                                   num_entries)) {
    print_error("\n%s %d: FAILED non-ranking INCREMENT %dth item '%d' in %s heap: %d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
   }

  /* Decrement a node without modifying its rank among the others */
  i = 3;
  test_array[i].value -= 1;
  err = binary_heap_modify(&heap, &test_array[i].node);
  if (err != BINARY_HEAP_ERR_OK) { 
    print_error("\n%s %d: Cannot DECREMENT %dth item '%d' in %s heap :%d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
   }
  if (test_binary_heap_verify_sort(&heap, test_array,
                                   (char *)__FUNCTION__,
                                   num_entries)) {
    print_error("\n%s %d: FAILED non-ranking DECREMENT %dth item '%d' in %s heap: %d",
                __FUNCTION__, __LINE__,
                i, test_array[i].value,
                heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
  }

  /* Increment a node so that it becomes greater than others */
  i = 4;
  test_array[i].value += 20;
  err = binary_heap_modify(&heap, &test_array[i].node);
  if (err != BINARY_HEAP_ERR_OK) { 
    print_error("\n%s %d: Cannot INCREMENT %dth item '%d' in %s heap :%d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  if (test_binary_heap_verify_sort(&heap, test_array,
                                   (char *)__FUNCTION__,
                                   num_entries)) {
    print_error("\n%s %d: FAILED ranking INCREMENT %dth item '%d' in %s heap",
                __FUNCTION__, __LINE__,
                i, test_array[i].value,
                heap.heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
  }

  
  /* Decrement a node so that it becomes greater than others */
  i = 5;
  test_array[i].value -= 20;
  err = binary_heap_modify(&heap, &test_array[i].node);
  if (err != BINARY_HEAP_ERR_OK) { 
    print_error("\n%s %d: Cannot DECREMENT %dth item '%d' in %s heap :%d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  if (test_binary_heap_verify_sort(&heap, test_array,
                                   (char *)__FUNCTION__, num_entries)) {
    print_error("\n%s %d: FAILED ranking DECREMENT %dth item '%d' in %s heap: %d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }


  /* INcrement a node so that it becomes the max */
  i = 0;
  test_array[i].value = 1000;
  err = binary_heap_modify(&heap, &test_array[i].node);
  if (err != BINARY_HEAP_ERR_OK) { 
    print_error("\n%s %d: Cannot MAXIMIZE %dth item '%d' in %s heap :%d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  if (test_binary_heap_verify_sort(&heap, test_array,
                                   (char *) __FUNCTION__,
                                   num_entries)) {
    print_error("\n%s %d: FAILED MAXIMIZING %dth item '%d' in %s heap",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max");
      local_fail++;
      goto out;
   }

  /* Decrement a node so that it becomes the min */  
  i = 6;
  test_array[i].value = 1;
  err = binary_heap_modify(&heap, &test_array[i].node);
  if (err != BINARY_HEAP_ERR_OK) { 
    print_error("\n%s %d: Cannot MINIMIZE %dth item '%d' in %s heap :%d",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }
  if (test_binary_heap_verify_sort(&heap, test_array,
                                   (char *) __FUNCTION__,
                                   num_entries)) {
    print_error("\n%s %d: FAILED MINIMIZING %dth item '%d' in %s heap",
                __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max");
    local_fail++;
    goto out;
   }


out:
  num_fail += local_fail;

  if (num_fail) {
    print_error("\n****Test '%s' in %s heap FAILED !!",
                __FUNCTION__, 
                heap_type == BINARY_HEAP_MIN ? "min": "max");
  } else {
    fprintf(stdout, COLOR_GREEN);
    fprintf(stdout, "\nTest '%s' in %s heap succeeded with %d items",
            __FUNCTION__, 
            heap_type == BINARY_HEAP_MIN ? "min": "max",
            num_entries);
    fprintf(stdout, COLOR_RESET);
  }
}

/**************************** H E A P   T O P *******************************/

void test_binary_heap_top(binary_heap_type_t heap_type)
{
  int entries[] =        {78, 24, 39, 3, 18, 99, 7, 15, 49, 31, 103, 65, 110};
  int sorted_ascend[]  = {3, 7, 15, 18, 24, 31, 39, 49, 65, 78, 99, 103, 110};
  int sorted_descend[] = {110, 103, 99, 78, 65, 49, 39, 31, 24, 18, 15, 7, 3};
  unsigned int num_entries = sizeof(entries) / sizeof(int);
  unsigned int i;
  struct int_array_st *test_array = malloc(num_entries * sizeof(*test_array));
  binary_heap_t heap;
  binary_heap_err_t err;
  uint32_t local_fail = 0;
  struct int_array_st *node;

  /* Initialize the test array */
  memset(test_array, 0, num_entries * sizeof(*test_array));
  for (i=0; i<num_entries; ++i) {
    test_array[i].value = entries[i];
  }

  /* create the heap */
  err = binary_heap_init(&heap,
                         heap_type, int_compare);
  if (err != BINARY_HEAP_ERR_OK) {
    print_error("\n%s %d: Cannot init heap type %s:%d",
                __FUNCTION__, __LINE__,
                heap_type == BINARY_HEAP_MIN ? "min": "max", err);
    local_fail++;
    goto out;
  }

  /* Let's insert the values. Note that they are already at random */
  for (i = 0; i < num_entries; i++) {
    err = binary_heap_insert(&heap, &test_array[i].node);
    if (err != BINARY_HEAP_ERR_OK) {
      print_error("\n%s %d: Cannot insert %dth item '%d' in %s heap :%d",
                  __FUNCTION__, __LINE__,
                  i, test_array[i].value,
                  heap.heap_type == BINARY_HEAP_MIN ? "min": "max", err);
      local_fail++;
      goto out;
    }
  }

  /*
   * Get the top of the heap and verify it
   */
  node = (struct int_array_st *)binary_heap_top(&heap);
  if ((heap_type == BINARY_HEAP_MIN && node && node->value  == sorted_ascend[0]) ||
      (heap_type == BINARY_HEAP_MAX && node && node->value  == sorted_descend[0])) {
    fprintf(stdout, "\n%s of heap is %d",
            heap_type == BINARY_HEAP_MIN ? "Min" : "Max", node->value);
  } else {
    num_fail++;
  }

out:
  num_fail += local_fail;

  if (num_fail) {
    print_error("\n****Test '%s' in %s heap FAILED !!",
                __FUNCTION__,
                heap_type == BINARY_HEAP_MIN ? "min": "max");
  } else {
    fprintf(stdout, COLOR_GREEN);
    fprintf(stdout, "\nTest '%s' in %s heap succeeded with %d items",
            __FUNCTION__,
            heap_type == BINARY_HEAP_MIN ? "min": "max",
            num_entries);
    fprintf(stdout, COLOR_RESET);
  }
}


/************ M A I N   F U N C N T I O N **********************/
int main(int argc, char *argv[])
{
  int i;
  int num_repeat = 10;
  fprintf(stdout, "\n*S T A R T I N G   B I N A R Y   H E A P   T E S T S*");
  srandom(DEFAULT_NUM_TEST_VALUES + 10);
  for (i = 0; i < num_repeat; i++) {
    uint32_t num_entries = random() % MAX_NUM_TEST_VALUES;
    printf("\nRepetition %d with %d entries", i, num_entries);
    test_binary_heap_insert(BINARY_HEAP_MIN, num_entries);
    test_binary_heap_insert(BINARY_HEAP_MAX, num_entries);
    test_binary_heap_delete(BINARY_HEAP_MIN, num_entries);
    test_binary_heap_delete(BINARY_HEAP_MAX, num_entries);
    test_binary_heap_insert_delete(BINARY_HEAP_MIN, num_entries);
    test_binary_heap_insert_delete(BINARY_HEAP_MAX, num_entries);
    printf("\n");
  }

  printf("\nTesting binary_heap_modify");
  test_binary_heap_modify(BINARY_HEAP_MIN);
  test_binary_heap_modify(BINARY_HEAP_MAX);

  printf("\n\nTesting binary_heap_top");
  test_binary_heap_top(BINARY_HEAP_MIN);
  test_binary_heap_top(BINARY_HEAP_MAX);

  if (num_fail) {
    print_error("\n\n  ***F A I L U R E S : %d !!***\n", num_fail);
  } else {
    fprintf(stdout, COLOR_GREEN);
    fprintf(stdout, "\n\n  **A L L   T E S T S   S U C C E E D E D**\n\n");
    fprintf(stdout, COLOR_RESET);
  }
  return 0;
}


