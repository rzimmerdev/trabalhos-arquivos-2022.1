//
// Created by rzimmerdev on 20/05/2022.
//

#include <stdio.h>

#ifndef T1_UTILS_H
#define T1_UTILS_H

#define BUFFER_SIZE 32

char *scan_word();
char *scan_word_quoted();

char *fscan_until(FILE *stream, char separator);

void binarioNaTela(char *nomeArquivoBinario);

#endif //T1_UTILS_H
