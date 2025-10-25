#include "estrutura.h"
#include "comandos.h"

int main() {  
    char comando[256];
    char arg1[256];
    
    inicializar_sistema_arquivos();
    
    printf("\n\nShell Luana e Maristela FS v1.0\n");
    printf("Digite 'ajuda' para ver os comandos disponíveis\n\n");
    
    while(1) {
        printf("\n %s> ", caminho_atual);
        
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }
        
        comando[strcspn(comando, "\n")] = 0;
        
        if (strlen(comando) == 0) {
            continue;
        }
        else if (sscanf(comando, "mkdir %255s", arg1) == 1) {
            criar_diretorio(arg1);
        }
        else if (sscanf(comando, "cd %255s", arg1) == 1) {
            mudar_diretorio(arg1);
        }
        else if (strcmp(comando, "pwd") == 0) {
            imprimir_diretorio_atual();
        }
        else if (sscanf(comando, "touch %255s", arg1) == 1) {
            criar_arquivo(arg1);
        }
        else if (sscanf(comando, "cat %255s", arg1) == 1) {
            mostrar_conteudo_arquivo(arg1);
        }
        else if (strcmp(comando, "ls") == 0) {
            listar_diretorio();
        }
        else if (sscanf(comando, "rm %255s", arg1) == 1) {
            remover_arquivo(arg1);
        }
        else if (strcmp(comando, "stat") == 0) {
            mostrar_status();
        }
        else if (strcmp(comando, "ajuda") == 0) {
            printf("\n\nComandos disponíveis:\n");
            printf("  mkdir <dir>      - Criar diretório\n");
            printf("  cd <caminho>     - Mudar diretório\n");
            printf("  pwd              - Mostrar diretório atual\n");
            printf("  touch <arquivo>  - Criar arquivo\n");
            printf("  cat <arquivo>    - Mostrar conteúdo do arquivo\n");
            printf("  ls               - Listar diretório\n");
            printf("  rm <arquivo/dir> - Remover arquivo/diretório\n");
            printf("  stat             - Mostrar status do sistema\n");
            printf("  ajuda            - Mostrar esta ajuda\n");
            printf("  sair             - Sair do shell\n");
        }
        else if (strcmp(comando, "sair") == 0) {
            break;
        }
        else {
            printf("\n Comando não encontrado: %s\n", comando);
        }
    }
    
    return 0; 
}