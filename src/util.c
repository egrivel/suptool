#include "common.h"
#include "util.h"

#define DEFAULT_THRESHOLD 66

int threshold = DEFAULT_THRESHOLD;

void util_set_threshold(int th) {
   threshold = th;
}

int util_get_threshold() {
   return threshold;
}
