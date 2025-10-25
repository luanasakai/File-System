
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

void mudar_diretorio(const char *caminho) {
    if (strcmp(caminho, "/") == 0) {
        strcpy(caminho_atual, "/");
        inode_atual = 1;
    }
    else if (strcmp(caminho, "..") == 0) {
        char *ultima_barra = strrchr(caminho_atual, '/');
        if (ultima_barra != caminho_atual) {
            *ultima_barra = '\0';
        }
        inode_atual = 1; 
    }
    else {
        FILE *diretorio_atual = fopen("fs/blocks/0.dat", "rb");
        ENTRADA_DIRETORIO entradas[8];
        fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, diretorio_atual);
        fclose(diretorio_atual);
        
        int encontrado = 0;
        for (int i = 0; i < 8; i++) {
            if (strcmp(entradas[i].nome_arquivo, caminho) == 0 && entradas[i].numero_inode != 0) {
                FILE *arquivo_inodes = fopen("fs/inodes.dat", "rb");
                INODE node;
                fseek(arquivo_inodes, entradas[i].numero_inode * sizeof(INODE), SEEK_SET);
                fread(&node, sizeof(INODE), 1, arquivo_inodes);
                fclose(arquivo_inodes);
                
                if (node.tipo == 2) {
                    strcat(caminho_atual, caminho);
                    strcat(caminho_atual, "/");
                    inode_atual = entradas[i].numero_inode;
                    encontrado = 1;
                }
                break;
            }
        }
        
        if (!encontrado) {
            printf("Erro: Diretório '%s' não encontrado\n", caminho);
        }
    }
}

void criar_diretorio(const char *nome_diretorio) {
    if (strlen(nome_diretorio) > TAMANHO_NOME_ARQUIVO - 1) {
        printf("Erro: Nome do diretório muito longo\n");
        return;
    }
    
    int novo_inode = encontrar_inode_livre();
    if (novo_inode == -1) {
        printf("Erro: Não há i-nodes livres disponíveis\n");
        return;
    }
    
    int novo_bloco = encontrar_bloco_livre();
    if (novo_bloco == -1) {
        printf("Erro: Não há blocos livres disponíveis\n");
        return;
    }

    FILE *arquivo_inodes = fopen("fs/inodes.dat", "r+b");
    INODE novo_inode_dir = {
        .tipo = 2,
        .tamanho = 2,
        .blocos[0] = novo_bloco
    };
    fseek(arquivo_inodes, novo_inode * sizeof(INODE), SEEK_SET);
    fwrite(&novo_inode_dir, sizeof(INODE), 1, arquivo_inodes);
    fclose(arquivo_inodes);

    FILE *bloco_dir = fopen("fs/blocks/0.dat", "r+b");
    ENTRADA_DIRETORIO entradas[8];
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, bloco_dir);

    int slot_livre = -1;
    for (int i = 2; i < 8; i++) {
        if (entradas[i].nome_arquivo[0] == '\0') {
            slot_livre = i;
            break;
        }
    }
    
    if (slot_livre == -1) {
        printf("Erro: Diretório está cheio\n");
        return;
    }

    strcpy(entradas[slot_livre].nome_arquivo, nome_diretorio);
    entradas[slot_livre].numero_inode = novo_inode;
    
    fseek(bloco_dir, 0, SEEK_SET);
    fwrite(entradas, sizeof(ENTRADA_DIRETORIO), 8, bloco_dir);
    fclose(bloco_dir);

    char caminho_bloco[256];
    sprintf(caminho_bloco, "fs/blocks/%d.dat", novo_bloco);
    FILE *novo_dir = fopen(caminho_bloco, "wb");
    
    ENTRADA_DIRETORIO novas_entradas[8] = {0};
    strcpy(novas_entradas[0].nome_arquivo, ".");
    novas_entradas[0].numero_inode = novo_inode;
    strcpy(novas_entradas[1].nome_arquivo, "..");
    novas_entradas[1].numero_inode = inode_atual;
    
    fwrite(novas_entradas, sizeof(ENTRADA_DIRETORIO), 8, novo_dir);
    fclose(novo_dir);

    FILE *espaco_livre = fopen("fs/freespace.dat", "r+b");
    unsigned char bitmap[MAX_BLOCOS];
    fread(bitmap, 1, MAX_BLOCOS, espaco_livre);
    bitmap[novo_bloco] = 1;
    fseek(espaco_livre, 0, SEEK_SET);
    fwrite(bitmap, 1, MAX_BLOCOS, espaco_livre);
    fclose(espaco_livre);
    
    printf("Diretório '%s' criado com sucesso\n", nome_diretorio);
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
