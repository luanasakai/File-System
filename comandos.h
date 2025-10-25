
/*TO-DO: 
    [] TROCAR DE DIRETORIO
    [] REMOVER ARQUIVOS E DIRETORIOS
    [] SAIR DE UM ARQUIVO
*/

int encontrar_inode_livre() {
    FILE *arquivo_inodes = fopen("fs/inodes.dat", "rb");
    INODE nodes[MAX_INODES];
    fread(nodes, sizeof(INODE), MAX_INODES, arquivo_inodes);
    fclose(arquivo_inodes);
    
    for (int i = 2; i < MAX_INODES; i++) {
        if (nodes[i].tipo == 0) {
            return i;
        }
    }
    return -1;
}

int encontrar_bloco_livre() {
    FILE *espaco_livre = fopen("fs/freespace.dat", "rb");
    unsigned char bitmap[MAX_BLOCOS];
    fread(bitmap, 1, MAX_BLOCOS, espaco_livre);
    fclose(espaco_livre);
    
    for (int i = 1; i < MAX_BLOCOS; i++) {
        if (bitmap[i] == 0) {
            return i;
        }
    }
    return -1;
}

void mostrar_status() {
    FILE *espaco_livre = fopen("fs/freespace.dat", "rb");
    unsigned char bitmap[MAX_BLOCOS];
    fread(bitmap, 1, MAX_BLOCOS, espaco_livre);
    fclose(espaco_livre);
    
    int blocos_livres = 0;
    for (int i = 0; i < MAX_BLOCOS; i++) {
        if (bitmap[i] == 0) blocos_livres++;
    }
    
    printf("Status do sistema de arquivos:\n");
    printf("Espaço livre: %d Bytes\n", blocos_livres * TAMANHO_BLOCO);
    printf("Blocos livres: %d Blocos\n", blocos_livres);
    printf("Tamanho do bloco: %d Bytes\n", TAMANHO_BLOCO);
}

void remover_arquivo(const char *nome_arquivo) {
    printf("\n Removido '%s'\n", nome_arquivo);
}

void listar_diretorio() {
    printf("\nTipo  Inode  Nome                Tamanho\n");
    printf("----  -----  ----                -------\n");
    
    FILE *diretorio_atual = fopen("fs/blocks/0.dat", "rb");
    ENTRADA_DIRETORIO entradas[8];
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, diretorio_atual);
    fclose(diretorio_atual);
    
    for (int i = 0; i < 8; i++) {
        if (entradas[i].nome_arquivo[0] != '\0') {
            FILE *arquivo_inodes = fopen("fs/inodes.dat", "rb");
            INODE node;
            fseek(arquivo_inodes, entradas[i].numero_inode * sizeof(INODE), SEEK_SET);
            fread(&node, sizeof(INODE), 1, arquivo_inodes);
            fclose(arquivo_inodes);
            
            char tipo = (node.tipo == 2) ? 'd' : 'f';
            printf("%c     %-6d %-18s %d\n", 
                   tipo, entradas[i].numero_inode, 
                   entradas[i].nome_arquivo, node.tamanho);
        }
    }
}

void mostrar_conteudo_arquivo(const char *nome_arquivo) {
    printf("Conteúdo de '%s':\n", nome_arquivo);
}

void criar_arquivo(char *nome_arquivo) {
    printf("Digite o conteúdo do arquivo (pressione Ctrl+Z para finalizar):\n");
    
    char conteudo[1024] = {0};
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strlen(conteudo) + strlen(buffer) < sizeof(conteudo) - 1) {
            strcat(conteudo, buffer);
        } else {
            printf("Buffer cheio!\n");
            break;
        }
    }

    FILE *arquivo = fopen(nome_arquivo, "w");
    if (arquivo) {
        fputs(conteudo, arquivo);
        fclose(arquivo);
        printf("Arquivo '%s' criado com sucesso!\n", nome_arquivo);
    } else {
        printf("Erro ao criar o arquivo '%s'\n", nome_arquivo);
    }
}

void imprimir_diretorio_atual() {
    printf("%s\n", caminho_atual);
}

/* 1. Procurar diretorio destino
   2. Salvar o atual (que vira anterior)
   3. Ir para o destino e salvar como atual

   LOCALIZAR DIRETORIO*/

// -------------------------------------------------------------
// Função para mudar de diretório (cd)
// Suporta: cd /, cd .., cd <nome>
// -------------------------------------------------------------
void mudar_diretorio(const char *caminho) {
    // -----------------------------------
    // Caso 1: cd /
    // -----------------------------------
    if (strcmp(caminho, "/") == 0) {
        strcpy(caminho_atual, "/");
        inode_atual = 1; // inode da raiz
        return;
    }

    // -----------------------------------
    // Caso 2: cd ..
    // -----------------------------------
    if (strcmp(caminho, "..") == 0) {
        if (strcmp(caminho_atual, "/") == 0)
            return; // já estamos na raiz, nada a fazer

        // Abrimos diretório atual
        char path[64];
        sprintf(path, "fs/blocks/%d.dat", inode_atual);
        FILE *dir = fopen(path, "rb");
        if (!dir) {
            printf("Erro: não foi possível abrir diretório atual\n");
            return;
        }

        ENTRADA_DIRETORIO entradas[8];
        fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
        fclose(dir);

        // Procuramos a entrada ".."
        unsigned char inode_pai = inode_atual; // default raiz
        for (int i = 0; i < 8; i++) {
            if (strcmp(entradas[i].nome_arquivo, "..") == 0) {
                inode_pai = entradas[i].numero_inode;
                break;
            }
        }

        // Atualiza inode atual
        inode_atual = inode_pai;

        // Atualiza caminho_atual removendo a última pasta
        if (strcmp(caminho_atual, "/") != 0) {
            char *ultima_barra = strrchr(caminho_atual, '/');
            if (ultima_barra != caminho_atual) {
                *ultima_barra = '\0'; // remove pasta
                strcat(caminho_atual, "/"); // garante barra
            } else {
                strcpy(caminho_atual, "/"); // volta para raiz
            }
        }
        return;
    }

    // -----------------------------------
    // Caso 3: cd <nome_diretorio>
    // -----------------------------------
    char path[64];
    sprintf(path, "fs/blocks/%d.dat", inode_atual);

    FILE *dir = fopen(path, "rb");
    if (!dir) {
        printf("Erro: não foi possível abrir diretório atual\n");
        return;
    }

    ENTRADA_DIRETORIO entradas[8];
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
    fclose(dir);

    int encontrado = 0;
    for (int i = 0; i < 8; i++) {
        if (strcmp(entradas[i].nome_arquivo, caminho) == 0 &&
            entradas[i].numero_inode != 0) {

            INODE node;
            FILE *arquivo_inodes = fopen("fs/inodes.dat", "rb");
            fseek(arquivo_inodes, entradas[i].numero_inode * sizeof(INODE), SEEK_SET);
            fread(&node, sizeof(INODE), 1, arquivo_inodes);
            fclose(arquivo_inodes);

            if (node.tipo == 2) { // é diretório
                // Remove barra extra se necessário
                if (strcmp(caminho_atual, "/") != 0)
                    strcat(caminho_atual, caminho);
                else
                    strcat(caminho_atual, caminho);

                // Garante barra no final
                if (caminho_atual[strlen(caminho_atual)-1] != '/')
                    strcat(caminho_atual, "/");

                inode_atual = entradas[i].numero_inode;
                encontrado = 1;
                break;
            }
        }
    }

    if (!encontrado)
        printf("Erro: Diretório '%s' não encontrado ou não é diretório\n", caminho);
}

void carregar_superbloco() {
    FILE *super = fopen("fs/superblock.dat", "rb");
    if (super) {
        SUPERBLOCO sb;
        fread(&sb, sizeof(SUPERBLOCO), 1, super);
        fclose(super);
    }
}

void inicializar_sistema_arquivos() {
    struct stat st;
    
    if (stat("fs", &st) == -1) {
        mkdir("fs", 0755);
    }
    
    if (stat("fs/superblock.dat", &st) == -1) {
        printf("Inicializando sistema de arquivos Luana e Maristela FS...\n");
        
        FILE *super = fopen("fs/superblock.dat", "wb");
        SUPERBLOCO sb = {
            .sistema_arquivos = "luana_e_maristela_fs",
            .tamanho_bloco = TAMANHO_BLOCO,
            .tamanho_particao = TAMANHO_PARTICAO,
            .total_blocos = MAX_BLOCOS,
            .total_inodes = MAX_INODES
        };
        fwrite(&sb, sizeof(SUPERBLOCO), 1, super);
        fclose(super);
        
        FILE *arquivo_inodes = fopen("fs/inodes.dat", "wb");
        INODE inodes[MAX_INODES] = {0};
        
        inodes[1].tipo = 2; 
        inodes[1].tamanho = 2; 
        inodes[1].blocos[0] = 0; 
        
        fwrite(inodes, sizeof(INODE), MAX_INODES, arquivo_inodes);
        fclose(arquivo_inodes);
        
        FILE *espaco_livre = fopen("fs/freespace.dat", "wb");
        unsigned char bitmap[MAX_BLOCOS] = {0};
        bitmap[0] = 1;
        fwrite(bitmap, 1, MAX_BLOCOS, espaco_livre);
        fclose(espaco_livre);
        
        mkdir("fs/blocks", 0755);
        
        FILE *bloco_raiz = fopen("fs/blocks/0.dat", "wb");
        ENTRADA_DIRETORIO entradas[8] = {0};

        strcpy(entradas[0].nome_arquivo, ".");
        entradas[0].numero_inode = 1;

        strcpy(entradas[1].nome_arquivo, "..");
        entradas[1].numero_inode = 1;
        
        fwrite(entradas, sizeof(ENTRADA_DIRETORIO), 8, bloco_raiz);
        fclose(bloco_raiz);
        
        printf("Sistema de arquivos inicializado com sucesso!\n");
    }
    
    carregar_superbloco();
}
