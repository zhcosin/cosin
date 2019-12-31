
#ifndef __COSIN_XML_UTILS_H_
#define __COSIN_XML_UTILS_H_

#include <stddef.h>

char* utils_strncpy (char *dest, const char *src, size_t length);
void* utils_malloc (size_t size);
void utils_free(void **p);
char* utils_strdup (const char *ch);
int utils_tolower (char *word);
int utils_clrspace (char *word);
int total_char_count(const char *str, char c);
int utils_free_char_vec(char ***dest, int size_vec);

#endif
