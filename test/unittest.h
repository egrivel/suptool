// Common include files
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

// Main test method, to be implemented by the test module
void test_it(void);

void unit_start(char *name);
void unit_description(char *descr);
void unit_step(char *step);
void unit_assert(bool assertion);
void unit_int_equal(int val1, int val2);
void unit_ptr_null(void *ptr);
void unit_ptr_non_null(void *ptr);
void unit_ptr_equal(void *val1, void *val2);
void unit_ptr_unequal(void *val1, void *val2);
void unit_file_exists(char *fname);
void unit_file_not_exists(char *fname);
void unit_file_content(char *fname, char *data);
void unit_file_remove(char *fname);
void unit_done();
