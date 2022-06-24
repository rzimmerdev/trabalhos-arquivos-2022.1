#include <stdio.h>

#ifndef T1_UTILS_H
#define T1_UTILS_H

#define BUFFER_SIZE 32


/* Scans a single word ended in ' ' (space character) or newline character.
*
* Returns:
*     char *: Initialized pointer to zero ended string
*/
char *scan_word();


/* Scans an input stream up until given separator character is found, or until EOF.
*
* Args:
*     FILE *stream: Stream to fscan characters from
*     char separator: Character to compare each scanned character to
*
* Returns:
*     char *: Initialized pointer to zero ended string
*/
char *fscan_until(FILE *stream, char separator);


/* Scans unlimited characters up until a quote character is found, then, scans and stores characters up until,
* new quotes are found.
*
* Returns:
*     char *: Initialized pointer to zero ended string corresponding to text inbetween quotes
*/
char *scan_word_quoted();


void binarioNaTela(char *nomeArquivoBinario);

#endif //T1_UTILS_H
