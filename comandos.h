<<<<<<< HEAD

/*TO-DO: 
=======
   /*TO-DO: 
>>>>>>> af41382 (Correção da criação de blocos nos inodes correspondentes)
    [ok] TROCAR DE DIRETORIO
    []. e .. nao funcionam
    [] REMOVER ARQUIVOS E DIRETORIOS
    [] SAIR DE UM ARQUIVO
<<<<<<< HEAD
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
    char tipo;

    sprintf(diretorio_atual, "fs/blocks/%d.dat", inode_atual);
    FILE *dir = fopen(diretorio_atual, "rb");
    if (!dir) {
        printf("Erro: não foi possível abrir diretório atual\n");
        return;
    }
    
    printf("\nTipo  Inode  Nome                Tamanho\n");
    printf("----  -----  ----                -------\n");
    
    ENTRADA_DIRETORIO entradas[8];
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
    fclose(dir);
    
    FILE *arquivo_inodes = fopen("fs/inodes.dat", "rb");
    if (!arquivo_inodes) {
        printf("Erro: não foi possível abrir arquivo de inodes\n");
        return;
    }

    for (int i = 0; i < 8; i++) {
        if (entradas[i].nome_arquivo[0] != '\0') {
            INODE node;
            fseek(arquivo_inodes, entradas[i].numero_inode * sizeof(INODE), SEEK_SET);
            fread(&node, sizeof(INODE), 1, arquivo_inodes);
            
            tipo = (node.tipo == 2) ? 'd' : 'f'; // d = diretório, f = arquivo
            printf("%c     %-6d %-18s %d\n", 
                   tipo, entradas[i].numero_inode, 
                   entradas[i].nome_arquivo, node.tamanho);
        }
    }

    fclose(arquivo_inodes);
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

void mudar_diretorio(const char *caminho) {
    unsigned char inode_pai;
    // -----------------------------------
    // Caso 0: cd . - permanece no diretório atual
    // -----------------------------------
    if (strcmp(caminho, ".") == 0) {
        return;
    }

    // -----------------------------------
    // Caso 1: cd / - vai para raiz
    // -----------------------------------
    if (strcmp(caminho, "/") == 0) {
        strcpy(caminho_atual, "/");
        inode_atual = 0; // inode da raiz
        return;
    }

    // -----------------------------------
    // Caso 2: cd .. - vai para diretório pai
    // -----------------------------------
    if (strcmp(caminho, "..") == 0) {
        if (strcmp(caminho_atual, "/") == 0) {
            // já está na raiz
            return;
        }

        sprintf(diretorio_atual, "fs/blocks/%d.dat", inode_atual);
        FILE *dir = fopen(diretorio_atual, "rb");
        if (!dir) {
            printf("Erro: não foi possível abrir diretório atual\n");
            return;
        }

        ENTRADA_DIRETORIO entradas[8];
        fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
        fclose(dir);

        // procura o inode do diretório pai (“..”)
        inode_pai = inode_atual;
        for (int i = 0; i < 8; i++) {
            if (strcmp(entradas[i].nome_arquivo, "..") == 0) {
                inode_pai = entradas[i].numero_inode;
                break;
            }
        }

        inode_atual = inode_pai;

        // Atualiza caminho_atual removendo a última pasta
        if (strcmp(caminho_atual, "/") != 0) {
            char *ultima_barra = strrchr(caminho_atual, '/');

            // caso "/usr/" → vira "/"
            if (ultima_barra == caminho_atual) {
                strcpy(caminho_atual, "/");
            } else if (ultima_barra) {
                *ultima_barra = '\0'; // corta no último '/'
                if (caminho_atual[strlen(caminho_atual) - 1] != '/')
                    strcat(caminho_atual, "/");
            }
        }
        return;
    }

    // -----------------------------------
    // Caso 3: cd <nome_diretorio>
    // -----------------------------------
    sprintf(diretorio_atual, "fs/blocks/%d.dat", inode_atual);
    FILE *dir = fopen(diretorio_atual, "rb");
    if (!dir) {
        printf("Erro: não foi possível abrir diretório atual\n");
=======
     */

    int encontrar_inode_livre() {
        FILE * arquivo_inodes = fopen("fs/inodes.dat", "rb");
        INODE nodes[MAX_INODES
        ];
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
        FILE * espaco_livre = fopen("fs/freespace.dat", "rb");
        unsigned char bitmap[MAX_BLOCOS
        ];
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
        FILE * espaco_livre = fopen("fs/freespace.dat", "rb");
        unsigned char bitmap[MAX_BLOCOS
        ];
    fread(bitmap, 1, MAX_BLOCOS, espaco_livre);
        fclose(espaco_livre);

        int blocos_livres = 0;
        for (int i = 0; i < MAX_BLOCOS; i++) {
            if (bitmap[i] == 0) {
                blocos_livres++;
            }
        }

        printf("Status do sistema de arquivos:\n");
        printf("Espaço livre: %d Bytes\n", blocos_livres * TAMANHO_BLOCO);
        printf("Blocos livres: %d Blocos\n", blocos_livres);
        printf("Tamanho do bloco: %d Bytes\n", TAMANHO_BLOCO);
    }

void remover_arquivo( char *nome_arquivo) {
    sprintf(diretorio_atual, "fs/blocks/%d.dat", inode_atual);
    FILE *dir = fopen(diretorio_atual, "r+b");
    if (!dir) {
        printf("Erro: não foi possível abrir o diretório atual\n");
>>>>>>> af41382 (Correção da criação de blocos nos inodes correspondentes)
        return;
    }

    ENTRADA_DIRETORIO entradas[8];
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
<<<<<<< HEAD
    fclose(dir);

    int encontrado = 0;
    for (int i = 0; i < 8; i++) {
        if (strcmp(entradas[i].nome_arquivo, caminho) == 0 &&
            entradas[i].numero_inode != 0) {

            INODE node;
            FILE *arquivo_inodes = fopen("fs/inodes.dat", "rb");
            if (!arquivo_inodes) {
                printf("Erro: não foi possível abrir inodes.dat\n");
                return;
            }

            fseek(arquivo_inodes, entradas[i].numero_inode * sizeof(INODE), SEEK_SET);
            fread(&node, sizeof(INODE), 1, arquivo_inodes);
            fclose(arquivo_inodes);

            if (node.tipo == 2) { // tipo 2 → diretório
                inode_atual = entradas[i].numero_inode;
                encontrado = 1;

                // Atualiza o caminho atual corretamente
                if (strcmp(caminho_atual, "/") != 0)
                    strcat(caminho_atual, caminho);
                else
                    strcat(caminho_atual, caminho);

                if (caminho_atual[strlen(caminho_atual) - 1] != '/')
                    strcat(caminho_atual, "/");

                break;
            }
        }
    }

    if (!encontrado) {
        printf("Erro: Diretório '%s' não encontrado ou não é diretório\n", caminho);
    }
}


void criar_diretorio(const char *nome_diretorio) {
    int novo_inode = 0, novo_bloco = 0;
    char caminho_bloco[256];

    if (strlen(nome_diretorio) > TAMANHO_NOME_ARQUIVO - 1) {
        printf("\nErro: Nome do diretório muito longo\n");
        return;
    }

    // --------------------------------------------------
    // Alocar inode e bloco livres
    // --------------------------------------------------
    novo_inode = encontrar_inode_livre();
    novo_bloco = encontrar_bloco_livre();

    if (novo_inode == -1 || novo_bloco == -1) {
        printf("Erro: Não há inode ou bloco livre disponível\n");
        return;
    }

    // --------------------------------------------------
    // Criar o inode do novo diretório
    // --------------------------------------------------
=======

    int encontrado = 0;
    int inode_arquivo = -1;

    // 🔹 1. Procura o arquivo/diretório 
    for (int i = 2; i < 8; i++) {
        if (strcmp(entradas[i].nome_arquivo, nome_arquivo) == 0 && encontrado == 0) {
            inode_arquivo = entradas[i].numero_inode;
            encontrado = 1;

            // Remove a entrada do diretório
            entradas[i].nome_arquivo[0] = '\0';
            entradas[i].numero_inode = 0;
        }
    }

    fclose(dir);

    if (!encontrado) {
        printf("Erro: '%s' não encontrado.\n", nome_arquivo);
        return;
    }

    // 🔹 2. Lê o inode do item encontrado
>>>>>>> af41382 (Correção da criação de blocos nos inodes correspondentes)
    FILE *arquivo_inodes = fopen("fs/inodes.dat", "r+b");
    if (!arquivo_inodes) {
        printf("Erro: não foi possível abrir inodes.dat\n");
        return;
    }

<<<<<<< HEAD
    INODE novo_inode_dir = {0};
    novo_inode_dir.tipo = 2;               // diretório
    novo_inode_dir.tamanho = 2;            // contém "." e ".."
    novo_inode_dir.blocos[0] = novo_bloco; // primeiro bloco de dados

    fseek(arquivo_inodes, novo_inode * sizeof(INODE), SEEK_SET);
    fwrite(&novo_inode_dir, sizeof(INODE), 1, arquivo_inodes);
    fclose(arquivo_inodes);

    // --------------------------------------------------
    // Adicionar entrada no diretório atual (pai)
    // --------------------------------------------------
    sprintf(diretorio_atual, "fs/blocks/%d.dat", inode_atual);
    FILE *dir_pai = fopen(diretorio_atual, "r+b");
    if (!dir_pai) {
        printf("Erro: não foi possível abrir diretório atual\n");
        return;
    }

    ENTRADA_DIRETORIO entradas[8] = {0};
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir_pai);

    int slot_livre = -1;
    for (int i = 2; i < 8; i++) { // posições 0 e 1 são "." e ".."
        if (entradas[i].nome_arquivo[0] == '\0') {
            slot_livre = i;
            break;
        }
    }

    if (slot_livre == -1) {
        printf("Erro: Diretório está cheio\n");
        fclose(dir_pai);
        return;
    }

    strcpy(entradas[slot_livre].nome_arquivo, nome_diretorio);
    entradas[slot_livre].numero_inode = novo_inode;

    fseek(dir_pai, 0, SEEK_SET);
    fwrite(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir_pai);
    fclose(dir_pai);

    // --------------------------------------------------
    // Criar o arquivo do novo diretório
    // --------------------------------------------------
    sprintf(caminho_bloco, "fs/blocks/%d.dat", novo_bloco);
    FILE *novo_dir = fopen(caminho_bloco, "wb");
    if (!novo_dir) {
        printf("Erro: não foi possível criar o arquivo do novo diretório\n");
        return;
    }

    ENTRADA_DIRETORIO novas_entradas[8] = {0};

    // "." aponta para ele mesmo
    strcpy(novas_entradas[0].nome_arquivo, ".");
    novas_entradas[0].numero_inode = novo_inode;

    // ".." aponta para o diretório pai
    strcpy(novas_entradas[1].nome_arquivo, "..");
    novas_entradas[1].numero_inode = inode_atual;

    fwrite(novas_entradas, sizeof(ENTRADA_DIRETORIO), 8, novo_dir);
    fclose(novo_dir);

    // --------------------------------------------------
    // Atualizar o bitmap de blocos (freespace.dat)
    // --------------------------------------------------
    FILE *espaco_livre = fopen("fs/freespace.dat", "r+b");
    if (espaco_livre) {
        unsigned char bitmap[MAX_BLOCOS];
        fread(bitmap, 1, MAX_BLOCOS, espaco_livre);
        bitmap[novo_bloco] = 1; // marca o bloco como usado
        fseek(espaco_livre, 0, SEEK_SET);
        fwrite(bitmap, 1, MAX_BLOCOS, espaco_livre);
        fclose(espaco_livre);
    } else {
        printf("\n Aviso: não foi possível atualizar freespace.dat\n");
    }

    printf("\n Diretório '%s' criado com sucesso (inode - %d | bloco - %d)\n",
           nome_diretorio, novo_inode, novo_bloco);
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
    
    /*1. Verifica se a pasta fs existe
     2.Se não existir, cria o diretório com permissões 
    755 (leitura/escrita/execução para dono, leitura/execução para outros)*/
    if (stat("fs", &st) == -1) {
        mkdir("fs", 0755);
    }
    
    //Se o arquivo superblock.dat não existir, significa que é a primeira execução
    if(stat("fs/superblock.dat", &st) == -1){
        //cria as estruturas basicas do sistema de arquivos
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
    
    }else{
=======
    INODE node;
    fseek(arquivo_inodes, inode_arquivo * sizeof(INODE), SEEK_SET);
    fread(&node, sizeof(INODE), 1, arquivo_inodes);

    // 🔹 3. Verifica se é diretório e se está vazio
    if (node.tipo == 'd') {
        char caminho_dir[64];
        sprintf(caminho_dir, "fs/blocks/%d.dat", inode_arquivo);

        FILE *subdir = fopen(caminho_dir, "rb");
        if (!subdir) {
            printf("Erro ao abrir subdiretório.\n");
            fclose(arquivo_inodes);
            return;
        }

        ENTRADA_DIRETORIO sub_entradas[8];
        fread(sub_entradas, sizeof(ENTRADA_DIRETORIO), 8, subdir);
        fclose(subdir);

        int vazio = 1;

        // 🔸 Verifica se há qualquer entrada válida além de "." e ".."
        for (int j = 2; j < 8; j++) {
            if (sub_entradas[j].nome_arquivo[0] != '\0' &&
                strlen(sub_entradas[j].nome_arquivo) > 0) {
                vazio = 0;
            }
        }

        if (!vazio) {
            printf("Erro: o diretório '%s' não está vazio.\n", nome_arquivo);
            return;
        }
    }

    // 🔹 4. Guarda blocos usados antes de limpar
    int blocos_usados[8];
    for (int i = 0; i < 8; i++) blocos_usados[i] = node.blocos[i];

    // 🔹 5. Libera o inode
    node.tipo = 0;
    node.tamanho = 0;
    for (int i = 0; i < 8; i++) node.blocos[i] = -1;

    fseek(arquivo_inodes, inode_arquivo * sizeof(INODE), SEEK_SET);
    fwrite(&node, sizeof(INODE), 1, arquivo_inodes);
    fclose(arquivo_inodes);

    // 🔹 6. Libera blocos no freespace.dat
    FILE *espaco = fopen("fs/freespace.dat", "r+b");
    if (espaco) {
        unsigned char bitmap[MAX_BLOCOS];
        fread(bitmap, 1, MAX_BLOCOS, espaco);

        for (int i = 0; i < 8; i++) {
            if (blocos_usados[i] != -1)
                bitmap[blocos_usados[i]] = 0;
        }

        fseek(espaco, 0, SEEK_SET);
        fwrite(bitmap, 1, MAX_BLOCOS, espaco);
        fclose(espaco);
    }

    // 🔹 7. Atualiza diretório pai
    dir = fopen(diretorio_atual, "r+b");
    if (!dir) {
        printf("Erro ao reabrir diretório atual.\n");
        return;
    }
    fseek(dir, 0, SEEK_SET);
    fwrite(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
    fclose(dir);

    printf("'%s' removido com sucesso.\n", nome_arquivo);
}




    void listar_diretorio() {
        char tipo;
        // DEBUG básico
        char cwd[
        512];
    getcwd(cwd, sizeof(cwd));
        printf("[DEBUG] CWD=%s inode_atual=%d\n", cwd, inode_atual);

        // valida inode_atual
        if (inode_atual < 0 || inode_atual >= MAX_INODES) {
            printf("Erro: inode_atual inválido: %d\n", inode_atual);
            return;
        }

        // Abre inodes para descobrir qual bloco contém este diretório
        FILE * arquivo_inodes = fopen("fs/inodes.dat", "rb");
        if (!arquivo_inodes) {
            perror("Erro ao abrir inodes.dat");
            return;
        }
        INODE cur;
        fseek(arquivo_inodes, inode_atual * sizeof(INODE), SEEK_SET);
        if (fread( & cur, sizeof(INODE), 1, arquivo_inodes) != 1) {
            printf("Erro lendo inode %d\n", inode_atual);
            fclose(arquivo_inodes);
            return;
        }
        fclose(arquivo_inodes);

        int bloco_idx = cur.blocos[0];
        if (bloco_idx < 0 || bloco_idx >= MAX_BLOCOS) {
            printf("Erro: bloco inválido no inode %d: %d\n", inode_atual, bloco_idx);
            return;
        }

        char bloco_path[
        256];
    snprintf(bloco_path, sizeof(bloco_path), "fs/blocks/%d.dat", bloco_idx);
        printf("[DEBUG] abrindo bloco do diretório: %s\n", bloco_path);
        FILE * dir = fopen(bloco_path, "rb");
        if (!dir) {
            perror("Erro ao abrir bloco do diretório");
            printf("Erro: não foi possível abrir diretório atual\n");
            return;
        }

        printf("\nTipo  Inode  Nome                Tamanho\n");
        printf("----  -----  ----                -------\n");

        ENTRADA_DIRETORIO entradas[
        8];
    if (fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir) != 8) {
            // pode ser menor, mas aceitaremos
        }
        fclose(dir);

        arquivo_inodes = fopen("fs/inodes.dat", "rb");
        if (!arquivo_inodes) {
            perror("Erro ao abrir inodes.dat");
            return;
        }

        for (int i = 0; i < 8; i++) {
            if (entradas[i].nome_arquivo[0] != '\0') {
                INODE node;
                int inode_num = entradas[i].numero_inode;
                if (inode_num < 0 || inode_num >= MAX_INODES) {
                    printf("? entrada aponta inode inválido: %d\n", inode_num);
                    continue;
                }
                fseek(arquivo_inodes, inode_num * sizeof(INODE), SEEK_SET);
                fread( & node, sizeof(INODE), 1, arquivo_inodes);

                tipo = (node.tipo == 2) ? 'd' : 'f';
                printf("%c     %-6d %-18s %d\n",
                        tipo, inode_num,
                        entradas[i].nome_arquivo, node.tamanho);
            }
        }

        fclose(arquivo_inodes);
    }

    void mostrar_conteudo_arquivo(const char *nome_arquivo) {
    printf("Conteúdo de '%s':\n", nome_arquivo);
    }

    void criar_arquivo(char  
        *nome_arquivo) { printf("Digite o conteúdo do arquivo (pressione Ctrl+Z para finalizar):\n");
        char conteudo[
        1024] = {0
        };
        char buffer[
        256]; while (fgets(buffer, sizeof(buffer), stdin)) {
            if (strlen(conteudo) + strlen(buffer) < sizeof(conteudo) - 1) {
                strcat(conteudo, buffer);
            } else {
                printf("Buffer cheio!\n");
                break;
            }
        }
        FILE * arquivo = fopen(nome_arquivo, "w");
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
    void mudar_diretorio(
    const char *caminho

    
        ) {
    // Caso 1: cd .
    if (strcmp(caminho, ".") == 0) {
            return;
        }

        // Caso 2: cd /
        if (strcmp(caminho, "/") == 0) {
            strcpy(caminho_atual, "/");
            inode_atual = 0;
            return;
        }

        // Caso 3: cd ..
        if (strcmp(caminho, "..") == 0) {
            // 1️⃣ Lê o inode atual
            FILE * arquivo_inodes = fopen("fs/inodes.dat", "rb");
            if (!arquivo_inodes) {
                printf("Erro: não foi possível abrir inodes.dat\n");
                return;
            }

            INODE inode_atual_info;
            fseek(arquivo_inodes, inode_atual * sizeof(INODE), SEEK_SET);
            fread( & inode_atual_info, sizeof(INODE), 1, arquivo_inodes);
            fclose(arquivo_inodes);

            // 2️⃣ Abre o bloco do diretório atual
            char bloco_path[
            256];
        sprintf(bloco_path, "fs/blocks/%d.dat", inode_atual_info.blocos[0]);

            FILE * dir = fopen(bloco_path, "rb");
            if (!dir) {
                printf("Erro: não foi possível abrir diretório atual\n");
                return;
            }

            ENTRADA_DIRETORIO entradas[
            8];
        fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
            fclose(dir);

            // 3️⃣ Descobre o inode do diretório pai ("..")
            int inode_pai = inode_atual;
            for (int i = 0; i < 8; i++) {
                if (strcmp(entradas[i].nome_arquivo, "..") == 0) {
                    inode_pai = entradas[i].numero_inode;
                    break;
                }
            }
            inode_atual = inode_pai;

            // 4️⃣ Atualiza o caminho corretamente
            int len = strlen(caminho_atual);

            // Remove barra final se existir
            if (len > 1 && caminho_atual[len - 1] == '/') {
                caminho_atual[len - 1] = '\0';
                len--;
            }
            // Encontra a última barra
            char *ultima_barra = strrchr(caminho_atual, '/');
            if (ultima_barra && ultima_barra != caminho_atual) {
                 * ultima_barra = '\0';
                strcat(caminho_atual, "/");
            } else {
                // Se já está na raiz
                strcpy(caminho_atual, "/");
            }

            return;
        }

        // Caso 4: cd <nome_diretorio>
        FILE * arquivo_inodes = fopen("fs/inodes.dat", "rb");
        if (!arquivo_inodes) {
            printf("Erro: não foi possível abrir inodes.dat\n");
            return;
        }

        INODE inode_info;
        fseek(arquivo_inodes, inode_atual * sizeof(INODE), SEEK_SET);
        fread( & inode_info, sizeof(INODE), 1, arquivo_inodes);
        fclose(arquivo_inodes);

        // Abre o bloco correspondente ao diretório atual
        char bloco_path[
        256];
    sprintf(bloco_path, "fs/blocks/%d.dat", inode_info.blocos[0]);

        FILE * dir = fopen(bloco_path, "rb");
        if (!dir) {
            printf("Erro: não foi possível abrir diretório atual\n");
            return;
        }

        ENTRADA_DIRETORIO entradas[
        8];
    fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir);
        fclose(dir);

        // Limpa o nome do caminho (remove \n, se existir)
        char nome_limpo[
        32];
    strcpy(nome_limpo, caminho);
        nome_limpo[strcspn(nome_limpo, "\n")] = '\0';

        // Procura o diretório destino
        int encontrado = 0;
        for (int i = 0; i < 8; i++) {
            if (strcmp(entradas[i].nome_arquivo, nome_limpo) == 0) {
                FILE * arquivo_inodes2 = fopen("fs/inodes.dat", "rb");
                if (!arquivo_inodes2) {
                    printf("Erro: não foi possível abrir inodes.dat\n");
                    return;
                }

                INODE node_destino;
                fseek(arquivo_inodes2, entradas[i].numero_inode * sizeof(INODE), SEEK_SET);
                fread( & node_destino, sizeof(INODE), 1, arquivo_inodes2);
                fclose(arquivo_inodes2);

                // Verifica se é diretório
                if (node_destino.tipo == 2) {
                    inode_atual = entradas[i].numero_inode;
                    encontrado = 1;

                    // Atualiza o caminho
                    if (strcmp(caminho_atual, "/") != 0) {
                        strcat(caminho_atual, nome_limpo);
                    } else {
                        sprintf(caminho_atual, "/%s", nome_limpo);
                    }

                    if (caminho_atual[strlen(caminho_atual) - 1] != '/') {
                        strcat(caminho_atual, "/");
                    }
                }
                break;
            }
        }

        if (!encontrado) {
            printf("Erro: Diretório '%s' não encontrado ou não é diretório\n", nome_limpo);
        }
    }

    void criar_diretorio(
    const char *nome_diretorio

    
        ) {
    int novo_inode = 0, novo_bloco = 0;
        char caminho_bloco[
        256];

    if (strlen(nome_diretorio) > TAMANHO_NOME_ARQUIVO - 1) {
            printf("\nErro: Nome do diretório muito longo\n");
            return;
        }

        // --------------------------------------------------
        // Alocar inode e bloco livres
        // --------------------------------------------------
        novo_inode = encontrar_inode_livre();
        novo_bloco = encontrar_bloco_livre();

        if (novo_inode == -1 || novo_bloco == -1) {
            printf("Erro: Não há inode ou bloco livre disponível\n");
            return;
        }

        // --------------------------------------------------
        // Criar o inode do novo diretório
        // --------------------------------------------------
        FILE * arquivo_inodes = fopen("fs/inodes.dat", "r+b");
        if (!arquivo_inodes) {
            printf("Erro: não foi possível abrir inodes.dat\n");
            return;
        }

        INODE novo_inode_dir = {0};
        novo_inode_dir.tipo = 2;               // diretório
        novo_inode_dir.tamanho = 2;            // contém "." e ".."
        novo_inode_dir.blocos[0] = novo_bloco; // primeiro bloco de dados

        fseek(arquivo_inodes, novo_inode * sizeof(INODE), SEEK_SET);
        fwrite( & novo_inode_dir, sizeof(INODE), 1, arquivo_inodes);
        fclose(arquivo_inodes);

        // --------------------------------------------------
        // Adicionar entrada no diretório atual (pai)
        // --------------------------------------------------
        // 🔹 CORREÇÃO: precisamos descobrir o bloco do diretório pai
        FILE * arquivo_inodes_atual = fopen("fs/inodes.dat", "rb");
        if (!arquivo_inodes_atual) {
            printf("Erro: não foi possível abrir inodes.dat\n");
            return;
        }

        INODE inode_pai;
        fseek(arquivo_inodes_atual, inode_atual * sizeof(INODE), SEEK_SET);
        fread( & inode_pai, sizeof(INODE), 1, arquivo_inodes_atual);
        fclose(arquivo_inodes_atual);

        // Agora abrimos o bloco correto do diretório pai
        sprintf(diretorio_atual, "fs/blocks/%d.dat", inode_pai.blocos[0]);
        FILE * dir_pai = fopen(diretorio_atual, "r+b");
        if (!dir_pai) {
            printf("Erro: não foi possível abrir diretório atual\n");
            return;
        }

        ENTRADA_DIRETORIO entradas[
        8] = {0
        };
        fread(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir_pai);

        int slot_livre = -1;
        for (int i = 2; i < 8; i++) { // posições 0 e 1 são "." e ".."
            if (entradas[i].nome_arquivo[0] == '\0') {
                slot_livre = i;
                break;
            }
        }

        if (slot_livre == -1) {
            printf("Erro: Diretório está cheio\n");
            fclose(dir_pai);
            return;
        }

        strcpy(entradas[slot_livre].nome_arquivo, nome_diretorio);
        entradas[slot_livre].numero_inode = novo_inode;

        fseek(dir_pai, 0, SEEK_SET);
        fwrite(entradas, sizeof(ENTRADA_DIRETORIO), 8, dir_pai);
        fclose(dir_pai);

        // --------------------------------------------------
        // Criar o arquivo do novo diretório (bloco .dat)
        // --------------------------------------------------
        sprintf(caminho_bloco, "fs/blocks/%d.dat", novo_bloco);
        FILE * novo_dir = fopen(caminho_bloco, "wb");
        if (!novo_dir) {
            printf("Erro: não foi possível criar o arquivo do novo diretório\n");
            return;
        }

        ENTRADA_DIRETORIO novas_entradas[
        8] = {0
        };

        // "." aponta para ele mesmo
        strcpy(novas_entradas[0].nome_arquivo, ".");
        novas_entradas[0].numero_inode = novo_inode;

        // ".." aponta para o diretório pai
        strcpy(novas_entradas[1].nome_arquivo, "..");
        novas_entradas[1].numero_inode = inode_atual;

        fwrite(novas_entradas, sizeof(ENTRADA_DIRETORIO), 8, novo_dir);
        fclose(novo_dir);

        // --------------------------------------------------
        // Atualizar o bitmap de blocos (freespace.dat)
        // --------------------------------------------------
        FILE * espaco_livre = fopen("fs/freespace.dat", "r+b");
        if (espaco_livre) {
            unsigned char bitmap[MAX_BLOCOS
            ];
        fread(bitmap, 1, MAX_BLOCOS, espaco_livre);
            bitmap[novo_bloco] = 1; // marca o bloco como usado
            fseek(espaco_livre, 0, SEEK_SET);
            fwrite(bitmap, 1, MAX_BLOCOS, espaco_livre);
            fclose(espaco_livre);
        } else {
            printf("\nAviso: não foi possível atualizar freespace.dat\n");
        }

        printf("\nDiretório '%s' criado com sucesso (inode - %d | bloco - %d)\n",
                nome_diretorio, novo_inode, novo_bloco);
    }

    void carregar_superbloco() {
        FILE * super = fopen("fs/superblock.dat", "rb");
        if (super) {
            SUPERBLOCO sb;
            fread( & sb, sizeof(SUPERBLOCO), 1, super);
            fclose(super);
        }
    }

    void inicializar_sistema_arquivos() {
        struct stat st;

        /*1. Verifica se a pasta fs existe
     2.Se não existir, cria o diretório com permissões 
    755 (leitura/escrita/execução para dono, leitura/execução para outros)*/
        if (stat("fs",  & st) == -1) {
            mkdir("fs", 0755);
        }

        //Se o arquivo superblock.dat não existir, significa que é a primeira execução
        if (stat("fs/superblock.dat",  & st) == -1) {
            //cria as estruturas basicas do sistema de arquivos
            printf("Inicializando sistema de arquivos Luana e Maristela FS...\n");

            FILE * super = fopen("fs/superblock.dat", "wb");
            SUPERBLOCO sb = {
            .sistema_arquivos = "luana_e_maristela_fs"
            ,
            .tamanho_bloco = TAMANHO_BLOCO
            ,
            .tamanho_particao = TAMANHO_PARTICAO
            ,
            .total_blocos = MAX_BLOCOS
            ,
            .total_inodes = MAX_INODES
        };
        fwrite( & sb, sizeof(SUPERBLOCO), 1, super);
        fclose(super);

        FILE * arquivo_inodes = fopen("fs/inodes.dat", "wb");
        INODE inodes[MAX_INODES
        ] = {0
        };

// Cria o inode 0 como diretório raiz
        inodes[0].tipo = 2;        // 2 = diretório
        inodes[0].tamanho = 2;     // contém '.' e '..'
        inodes[0].blocos[0] = 0;   // aponta para o bloco 0 (raiz)

        fwrite(inodes, sizeof(INODE), MAX_INODES, arquivo_inodes);
        fclose(arquivo_inodes);

        FILE * espaco_livre = fopen("fs/freespace.dat", "wb");
        unsigned char bitmap[MAX_BLOCOS
        ] = {0
        };
        bitmap[0] = 1;
        fwrite(bitmap, 1, MAX_BLOCOS, espaco_livre);
        fclose(espaco_livre);

        mkdir("fs/blocks", 0755);

        FILE * bloco_raiz = fopen("fs/blocks/0.dat", "wb");
        ENTRADA_DIRETORIO entradas[
        8] = {0
        };

        strcpy(entradas[0].nome_arquivo, ".");
        entradas[0].numero_inode = 0;

        strcpy(entradas[1].nome_arquivo, "..");
        entradas[1].numero_inode = 0;

        fwrite(entradas, sizeof(ENTRADA_DIRETORIO), 8, bloco_raiz);
        fclose(bloco_raiz);

        printf("Sistema de arquivos inicializado com sucesso!\n");

    }

    
        else{
>>>>>>> af41382 (Correção da criação de blocos nos inodes correspondentes)
        //Se ja tiver sido incializado, inicializa o caminho e o inode
        strcpy(caminho_atual, "/");
        inode_atual = 0; // inode da raiz
    }

    carregar_superbloco();
<<<<<<< HEAD
}
=======
}
>>>>>>> af41382 (Correção da criação de blocos nos inodes correspondentes)
