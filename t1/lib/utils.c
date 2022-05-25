#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "utils.h"


char *scan_word() {

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

char *scan_word_quoted() {

    int size = 0;
    char *word = malloc(size * sizeof(char));
    char current_char;

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