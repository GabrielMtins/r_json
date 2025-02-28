# r\_json

## O que é r\_json?

r\_json é uma biblioteca para leitura de arquivos json feita em C89,
sem dependências da biblioteca padrão
e licenciada sob GPLv3(+later). O objetivo da biblioteca é ser uma biblioteca
simples e direta que possa ser utilizada junto com outros programas software
livre.

## Características

A r\_json manuseia sua própria memória. Para isso, o usuário terá que lhe fornecer
um bloco de memória junto com o tamanho em bytes para trabalhar. Com isso,
a biblioteca manuseará sua própria memória, sem utilizar malloc e free. O bloco
de memória deverá ser liberado pelo próprio usuário.

A vantagem disso é que a biblioteca poderá ser utilizada com ambientes que
utilizam sua própria alocação de memória (como em motores de jogos, sistemas
baixo nível), além de garantir a não-fragmentação da memória. 

## Documentação

A r\_json é uma biblioteca pequena, com pouquíssimas funções. Todas elas estão
descritas através de comentários no próprio header da biblioteca, com seus
valores de retornos esperados e suas funções. Também está
descrito membros das estruturas.

## Exemplo de utilização

Aqui está um exemplo de utilização da biblioteca:

```c
#include <stdio.h>
#include <stdlib.h>

#include "r_json.h"

/* String a ser lida. */
static const char *parse_string =
"{\n"
"	\"x\":2.1,\n"
"	\"y\":2.4\n"
"}\n";

int main(void){
    size_t memory_size = 1024 * 1024 * 4; /* 4MB de memória */
    /* Este exemplo usa malloc, mas pode-se utiliza uma array da stack,
     * ou até mesmo outro alocador de memória do seu interesse. */
    void *memory_block = malloc(memory_size);
    rjs_parser_t parser;

    /* Cria um parser com a memória desejada. */
    rjs_create_parser(&parser, memory_block, memory_size);

    /* Lê a string adequadamente. */
    if(rjs_parse_string(&parser, parse_string)){
        rjs_key_t *key;
        const rjs_object_t *obj;

        /* Retorna o objeto principal. */
        obj = rjs_get_main_object(&parser);

        /* Retorna a chave de nome desejado. */
        key = rjs_get_key(obj, "x");

        /* Verifica o tipo da chave. */
        if(rjs_istype(key, RJS_KEY_NUMBER))
            printf("x: %f\n", rjs_get_vnumber(key)); /* Imprime utilizando a função de retornar o valor da chave. */

        key = rjs_get_key(obj, "y");

        if(rjs_istype(key, RJS_KEY_NUMBER))
            printf("y: %f\n", rjs_get_vnumber(key));
    }
    else{
        /* Caso haja erro, imprima o erro. */
        printf("%s\n", rjs_get_error(&parser));
    }

    free(memory_block);

    return 0;
}
```

## Vantagens e Desvantagens

A r\_json foi uma biblioteca pensada para ser simples e facilmente integrável
em outros projetos. Por isso, ela utiliza o padrão C89 e também não tem dependências
da biblioteca padrão.

Porém, ela possui algumas desvantagens devido a sua simplicidade. Como o objetivo
da biblioteca é apenas fornecer um leitor de arquivos json, os dados armazenados
estão em listas encadeadas. Por isso, acessar dados em aplicações de tempo real
diretamente da r\_json é uma desvantagem, já que a complexidade de busca será O(n).
Idealmente, a biblioteca será utilizada apenas para decodificar a estrutura json,
e os dados serão posteriormente armazenados em estruturas de dados adequado pelo
usuário da biblioteca.
