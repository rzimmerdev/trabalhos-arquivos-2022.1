#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "utils.h"


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


char *scan_quote_string() {
	/*
	*	Use essa função para ler um campo string delimitado entre aspas (").
	*	Chame ela na hora que for ler tal campo. Por exemplo:
	*
	*	A entrada está da seguinte forma:
	*		nomeDoCampo "MARIA DA SILVA"
	*
	*	Para ler isso para as strings já alocadas str1 e str2 do seu programa, você faz:
	*		scanf("%s", str1); // Vai salvar nomeDoCampo em str1
	*		scan_quote_string(str2); // Vai salvar MARIA DA SILVA em str2 (sem as aspas)
	*
	*/
	char current_char;

	while((current_char = getchar()) != EOF && isspace(current_char)); // ignorar espaços, \r, \n...

    int size = 1;
    char *word = malloc(size * sizeof(char));
    word[0] = '\0';

	if(current_char == 'N' || current_char == 'n') { // campo NULO
		getchar(); getchar(); getchar(); // ignorar o "ULO" de NULO.
	} else if(current_char == '\"') {
        while (scanf("%c", &current_char) == 1 && current_char != '"') {
            if (current_char != '\r') {
                word = realloc(word, ++size * sizeof(char));
                word[size - 2] = current_char;
            }
        }
        
        word[size - 1] = '\0';
	} else if(current_char != EOF){
        ungetc(current_char, stdin); // vc tá tentando ler uma string que não tá entre aspas! Fazer leitura normal %s então, pois deve ser algum inteiro ou algo assim...
		free(word);
        word = scan_word();
	} 

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