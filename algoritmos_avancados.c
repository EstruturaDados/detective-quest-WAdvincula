/*
 Detective Quest - Sistema de pistas, suspeitos e julgamento
 Autor: (implementação exemplo)
 Compilar: gcc -std=c11 -O2 -o detective detective.c
 Executar: ./detective
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ---------- Estruturas ---------- */

/* Nó da árvore binária de salas (mansão) */
typedef struct Sala {
    char *nome;
    struct Sala *esq;
    struct Sala *dir;
} Sala;

/* Nó da BST de pistas coletadas */
typedef struct PistaNode {
    char *pista;
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

/* Entrada da tabela hash (encadeamento separado) */
typedef struct HashEntry {
    char *pista;
    char *suspeito;
    struct HashEntry *proximo;
} HashEntry;

/* Tabela hash com tamanho fixo */
#define HASH_SIZE 101
HashEntry *tabelaHash[HASH_SIZE] = { NULL };

/* ---------- Protótipos ---------- */

/* Funções exigidas com comentários explicativos */
Sala* criarSala(const char *nome); /* cria dinamicamente um cômodo */
void explorarSalas(Sala *atual); /* navega pela árvore e ativa o sistema de pistas */
PistaNode* inserirPista(PistaNode *raiz, const char *pista); /* insere pista na BST ordenada */
PistaNode* adicionarPista(PistaNode *raiz, const char *pista); /* alias para inserirPista */
void inserirNaHash(const char *pista, const char *suspeito); /* insere associação pista->suspeito */
char* encontrarSuspeito(const char *pista); /* consulta suspeito correspondente a uma pista */
void verificarSuspeitoFinal(PistaNode *colecao); /* conduz à fase de julgamento final */

/* Funções utilitárias */
const char* pistaPorSala(const char *nome);
unsigned long hash_djb2(const char *str);
void liberarSalas(Sala *r);
void liberarPistas(PistaNode *r);
void liberarHash();
void listarPistasInOrder(PistaNode *r);
int contarPistasParaSuspeito(PistaNode *r, const char *suspeito);
int pistaJaExiste(PistaNode *r, const char *pista);

/* Variável global: coleção de pistas */
PistaNode *colecaoPistas = NULL;

/* ---------- Implementação ---------- */

/*
 criarSala()
 Cria dinamicamente um nó Sala com o nome fornecido.
 O nome é duplicado com strdup para memória segura.
*/
Sala* criarSala(const char *nome) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        fprintf(stderr, "Erro de memória ao criar sala.\n");
        exit(EXIT_FAILURE);
    }
    s->nome = strdup(nome);
    s->esq = s->dir = NULL;
    return s;
}

/*
 explorarSalas()
 Função interativa que permite ao jogador navegar a mansão.
 A cada entrada em sala, verifica se existe pista associada e a coleta (na BST)
 Entrada: ponteiro para a sala atual (padrão: root).
 Comandos:
   'e' - ir para a esquerda (se existir)
   'd' - ir para a direita (se existir)
   's' - sair da exploração (encerra)
*/
void explorarSalas(Sala *atual) {
    Sala *cursor = atual;
    char comando[10];

    printf("\n--- Início da exploração da mansão ---\n");
    while (cursor) {
        printf("\nVocê está na sala: %s\n", cursor->nome);
        const char *pista = pistaPorSala(cursor->nome);
        if (pista) {
            printf("Você encontrou uma pista: \"%s\"\n", pista);
            if (!pistaJaExiste(colecaoPistas, pista)) {
                colecaoPistas = inserirPista(colecaoPistas, pista);
                printf("Pista coletada e armazenada.\n");
            } else {
                printf("Você já havia coletado essa pista antes; não foi duplicada.\n");
            }
        } else {
            printf("Nenhuma pista visível nesta sala.\n");
        }

        /* Exibir opções de movimento */
        printf("\nEscolha uma ação: (e) esquerda | (d) direita | (s) sair e julgar\n> ");
        if (!fgets(comando, sizeof(comando), stdin)) {
            /* entrada falhou */
            break;
        }
        /* pega primeiro caractere não branco */
        char c = '\0';
        for (int i = 0; comando[i]; ++i) {
            if (!isspace((unsigned char)comando[i])) { c = tolower((unsigned char)comando[i]); break; }
        }

        if (c == 'e') {
            if (cursor->esq) {
                cursor = cursor->esq;
            } else {
                printf("Não há sala à esquerda.\n");
            }
        } else if (c == 'd') {
            if (cursor->dir) {
                cursor = cursor->dir;
            } else {
                printf("Não há sala à direita.\n");
            }
        } else if (c == 's') {
            printf("\nVocê decidiu terminar a exploração.\n");
            break;
        } else {
            printf("Comando inválido. Use 'e', 'd' ou 's'.\n");
        }
    }
    printf("\n--- Exploração finalizada ---\n");
}

/*
 inserirPista()
 Insere uma nova pista (string) na árvore de busca binária (ordenada lexicograficamente).
 Retorna a raiz possivelmente atualizada.
 Se a pista já existir, não insere duplicatas (retorna a árvore original).
*/
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (!raiz) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) { fprintf(stderr, "Erro de memória ao inserir pista.\n"); exit(EXIT_FAILURE); }
        n->pista = strdup(pista);
        n->esq = n->dir = NULL;
        return n;
