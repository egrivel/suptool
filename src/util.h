#ifndef _UTIL_INCLUDED
#define _UTIL_INCLUDED

// Set the threshold at which a pixel is defined as visible
// The threshold is a value from 0 to 100, default is 66

void util_set_threshold(int rate);
int util_get_threshold();

#endif /* _UTIL_INCLUDED */
