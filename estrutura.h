#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

// Tamanhos maximos
#define TAMANHO_BLOCO 128
#define TAMANHO_PARTICAO 10240
#define MAX_BLOCOS 80
#define MAX_INODES 64
#define TAMANHO_NOME_ARQUIVO 14
#define TAMANHO_INODE 32

// Estrturas de dados dos componentes do sistema
typedef struct sEntrada_Diretorio {
    char nome_arquivo[TAMANHO_NOME_ARQUIVO];
    char numero_inode;
} ENTRADA_DIRETORIO;

typedef struct sInode {
    char tipo; 
    char tamanho;
    char blocos[10]; 
    char indireto_simples;
    char tempo_criacao;
    char tempo_modificacao;
} INODE;

typedef struct sSuperbloco {
    char sistema_arquivos[30];
    int tamanho_bloco;
    int tamanho_particao;
    int total_blocos;
    int total_inodes;
} SUPERBLOCO;

// Variaveis globais
char caminho_atual[256];
char diretorio_atual[64];
char bloco_atual[265];
char inode_atual;

// Prototipo de funcoes
int encontrar_inode_livre();
int encontrar_bloco_livre();
void mostrar_status();
void remover_arquivo(char *nome_arquivo);
void listar_diretorio();
void mostrar_conteudo_arquivo(const char *nome_arquivo);
int criar_entrada_diretorio(int inode_diretorio, const char *nome, int inode_destino);
void criar_arquivo(char *nome_arquivo);
void imprimir_diretorio_atual();
void mudar_diretorio(const char *caminho);
void criar_diretorio(const char *nome_diretorio);
void carregar_superbloco();
void inicializar_sistema_arquivos();
