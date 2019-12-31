
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xml/utils.h"

char* utils_strncpy (char *dest, const char *src, size_t length)
{
	strncpy (dest, src, length);
	dest[length] = '\0';
	return dest;
}

void* utils_malloc (size_t size)
{
	void *ptr = NULL;

	if (size == 0)
		return NULL;

	ptr = malloc (size);

	if (ptr != NULL)
		memset (ptr, 0, size);

	return ptr;
}

void utils_free(void **p)
{
	if (p == NULL || *p == NULL)
		return;

	free(*p);
	*p = NULL;
}

char* utils_strdup (const char *ch)
{
	char *copy;
	size_t length;

	if (ch == NULL)
		return NULL;
	length = strlen (ch);
	copy = (char *) utils_malloc (length + 1);
	if (copy==NULL)
		return NULL;
	utils_strncpy (copy, ch, length);
	return copy;
}

int utils_tolower (char *word)
{
	size_t i;
	size_t len = 0;

	if (word == NULL)
		return -1;

	len = strlen(word);

	for (i = 0; i <= len - 1; i++)
	{
		if ('A' <= word[i] && word[i] <= 'Z')
			word[i] += 32;
	}

	return 0;
}

int utils_clrspace (char *word)
{
	char *pbeg;
	char *pend;
	size_t len;

	if (word == NULL)
		return -1;

	len = strlen (word);
	if (len == 0)
		return 0;

	pbeg = word;
	while (*pbeg != '\0' && isspace(*pbeg))
		++pbeg;

	pend = word + len - 1;
	while (pend > word && isspace(*pend))
	{
		if (pend < pbeg)
		{
			*word = '\0';
			return 0;
		}

		--pend;
	}

	/* Add terminating NULL only if we've cleared room for it */
	if (pend + 1 <= word + (len - 1))
		pend[1] = '\0';

	if (pbeg != word)
		memmove (word, pbeg, pend - pbeg + 2);

	return 0;
}

int total_char_count(const char *str, char c)
{
	int count = 0;

	if (str == NULL)
		return 0;

	for (; *str != '\0'; ++str)
	{
		if (*str == c)
			++count;
	}
	
	return count;
}

int utils_free_char_vec(char ***dest, int size_vec)
{
	int i = 0;

	if (dest == NULL || size_vec < 0)
		return -1;

	if (size_vec == 0)
		return 0;

	for (i = 0; i < size_vec; ++i)
	{
		utils_free(*dest + i);
	}
	
	utils_free(*dest);

	return 0;
}