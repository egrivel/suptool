#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "unittest.h"

bool gl_verbose = false;
char *gl_current_test = NULL;
char *gl_current_step = NULL;
bool gl_test_success = true;

int main(int argc, char *argv[]) {
   int i;
   for (i = 1; i < argc; i++) {
      if (!strcmp(argv[i], "-v")) {
         gl_verbose = true;
      }
   }
   test_it();
   return 0;
}

void unit_start(char *name) {
   printf("UNITTEST: %s: ", name);
   if (gl_verbose) {
      printf("\n");
   }
   gl_current_test = name;
   gl_current_step = NULL;
   gl_test_success = true;
}

/*
 * Not doing anything with the description so far
 */
void unit_description(char *descr) {
}

void unit_step(char *step) {
   if ((gl_current_step != NULL) && gl_verbose) {
      // previous step succeeded
      printf("OK\n");
   }
   if (gl_verbose) {
      printf("   STEP %s: ", step);
   }
   gl_current_step = step;
}

void unit_assert(bool assertion) {
   if (gl_current_step != NULL) {
      if (gl_verbose) {
         if (assertion) {
            printf("OK\n");
         } else {
            printf("FAILED\n");
            gl_test_success = false;
         }
      } else if (!assertion) {
         if (gl_test_success) {
            // first failing step
            printf("FAILED\n");
         }
         printf("   STEP %s: FAILED\n", gl_current_step);
         gl_test_success = false;         
      }
      gl_current_step = NULL;
   }
}

void unit_message(bool success, char *fail_text) {
   if (gl_current_step != NULL) {
      if (gl_verbose) {
         if (success) {
            printf("OK\n");
         } else {
            printf("FAILED: %s\n", fail_text);
            gl_test_success = false;
         }
      } else if (!success) {
         if (gl_test_success) {
            // first failing step
            printf("FAILED\n");
         }
         printf("   STEP %s: FAILED: %s\n", gl_current_step, fail_text);
         gl_test_success = false;         
      }
      gl_current_step = NULL;
   }
}

void unit_int_equal(int val1, int val2) {
   bool success = (val1 == val2);
   char fail_text[256];
   sprintf(fail_text, "%d != %d", val1, val2);
   unit_message(success, fail_text);
}

void unit_ptr_null(void *ptr) {
   bool success = (ptr == NULL);
   char *fail_text = "pointer is not NULL";
   unit_message(success, fail_text);
}

void unit_ptr_non_null(void *ptr) {
   bool success = (ptr != NULL);
   char *fail_text = "pointer is NULL";
   unit_message(success, fail_text);
}

void unit_ptr_equal(void *val1, void *val2) {
   bool success = (val1 == val2);
   char *fail_text = "pointers are different";
   unit_message(success, fail_text);
}

void unit_ptr_unequal(void *val1, void *val2) {
   bool success = (val1 != val2);
   char *fail_text = "pointers are the same";
   unit_message(success, fail_text);
}

void unit_file_exists(char *fname) {
   struct stat file_stat;
   bool success = (stat(fname, &file_stat) >= 0);
   char *fail_text = "file does not exist";
   unit_message(success, fail_text);
}

void unit_file_not_exists(char *fname) {
   struct stat file_stat;
   bool success = (stat(fname, &file_stat) < 0);
   char *fail_text = "file exists";
   unit_message(success, fail_text);
}

void unit_file_content(char *fname, char *content) {
   char *buffer = malloc(strlen(content) + 10);
   memset(buffer, 0, strlen(content) + 10);
   FILE *fp = fopen(fname, "rt");
   fread(buffer, 1, strlen(content) + 8, fp);
   fclose(fp);
   bool success = !strcmp(buffer, content);
   char *fail_text = malloc(2 * strlen(buffer) + 20);
   sprintf(fail_text, "'%s' vs. '%s'", content, buffer);
   unit_message(success, fail_text);
   free(fail_text);
   free(buffer);
}

void unit_file_remove(char *fname) {
   unlink(fname);
}

void unit_done() {
   if ((gl_current_step != NULL) && gl_verbose) {
      printf("OK\n");
   }
   if (!gl_verbose && gl_test_success) {
      // all steps were fine
      printf("OK");
   }
   printf("\n");

   gl_current_test = NULL;
}
