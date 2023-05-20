#ifndef UTILITIES_HPP_INCLUDED
#define UTILITIES_HPP_INCLUDED

#include <math.h>
#include <stdbool.h>

#define Safety_calloc(size_of_memory) _Safety_calloc (size_of_memory, __LINE__)


void *_Safety_calloc (size_t size_of_memory, int line);
bool is_equal (double number_1, double number_2);
void *recalloc(void *ptr, size_t num, size_t size);
int hash8 (const char *message);
#endif