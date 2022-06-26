#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "utils.h"

// TODO: Add more comments


char *scan_word() {
    /* Scans a single word ended in ' ' (space character) or newline character.
     *
     * Returns:
     *     char *: Initialized pointer to zero ended string
     */
    int size = 0;
    char *word = malloc(size * sizeof(char));
    char current_char;

    while (scanf("%c", &current_char) == 1 && current_char != '\n' && current_char != ' ') {
        if (current_char != '\r') {
            word = realloc(word, ++size * sizeof(char));
            word[size - 1] = current_char;
        }
    }

    word = realloc(word, (size + 1) * sizeof(char));
    word[size] = '\0';
    return word;
}



char *fscan_until(FILE *stream, char separator) {
    /* Scans an input stream up until given separator character is found, or until EOF.
     *
     * Args:
     *     FILE *stream: Stream to fscan characters from
     *     char separator: Character to compare each scanned character to
     *
     * Returns:
     *     char *: Initialized pointer to zero ended string
     */
    int buffer = BUFFER_SIZE, i = 0;
    char *ptr = malloc(sizeof(char) * buffer);

    char current_char;
    while (fscanf(stream, "%c", &current_char) != EOF && current_char != '\n' && current_char != separator) {
        if (current_char != '\r') {
            if (buffer <= i) {
                buffer += BUFFER_SIZE;
                ptr = realloc(ptr, sizeof(char) * buffer);
            }
            ptr[i++] = current_char;
        }
    }

    if (current_char == EOF)
        ungetc(current_char, stream);

    ptr = realloc(ptr, (i + 1) * sizeof(char));
    ptr[i] = '\0';

    return ptr;
}


char *scan_word_quoted() {
    /* Scans unlimited characters up until a quote character is found, then, scans and stores characters up until,
     * new quotes are found.
     *
     * Returns:
     *     char *: Initialized pointer to zero ended string corresponding to text inbetween quotes
     */
    int size = 0;
    char *word = malloc(size * sizeof(char));
    char current_char = getchar();
    if (current_char == 'N') {
        word = realloc(word, 1 * sizeof(char));
        word[0] = '\0';
        getchar(); getchar(); getchar();
    } ungetc(current_char, stdin);

    while ((current_char = getchar()) != '"');

    while (scanf("%c", &current_char) == 1 && current_char != '"') {
        if (current_char != '\r') {
            word = realloc(word, ++size * sizeof(char));
            word[size - 1] = current_char;
        }
    }

    word = realloc(word, (size + 1) * sizeof(char));
    word[size] = '\0';
    
    return word;
}


void binarioNaTela(char *nomeArquivoBinario) { /* Você não precisa entender o código dessa função. */

    /* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo anteriormente.
    *  Ela vai abrir de novo para leitura e depois fechar (você não vai perder pontos por isso se usar ela). */

    unsigned long i, cs;
    unsigned char *mb;
    size_t fl;
    FILE *fs;
    if(nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
        fprintf(stderr, "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome certo? Você lembrou de fechar ele com fclose depois de usar?\n");
        return;
    }
    fseek(fs, 0, SEEK_END);
    fl = ftell(fs);
    fseek(fs, 0, SEEK_SET);
    mb = (unsigned char *) malloc(fl);
    fread(mb, 1, fl, fs);

    cs = 0;
    for(i = 0; i < fl; i++) {
        cs += (unsigned long) mb[i];
    }
    printf("%lf\n", (cs / (double) 100));
    free(mb);
    fclose(fs);
}