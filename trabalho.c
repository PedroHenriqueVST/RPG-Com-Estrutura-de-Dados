/*
  Aluno: Pedro Henrique Vieira dos Santos
  Matrícula: 202403047
  Engenharia de computção
*/


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>

/* ===== STRUCTS E CABEÇALHOS AQUI ===== */
typedef struct No {
    char *nome; 
    char *classe; 
    
    int nivel;
    int dado;
    int iniciativa;
    int forca;
    int destreza;
    int constituicao_base; 
    int sabedoria;

    int vida_atual;

    struct No* prox;
} No;

// ------------- Funções utilitárias e de criação -------------
No* criar_no(char *nome, char *classe, int nivel, int dado); 
void calcular_atributos(No* novo);
No* copiar_no(No* p);
void free_node(No *p);
void free_list(No **headRef);

// ------------- Funções de lista -------------
void inserir_no_comeco(No **lista, int classeId, char *nome);
void inserir_no_fim(No** lista, No* novo);
void exibir_lista(No *p);

// ------------- Ordenação (MergeSort usado para montar ordem) -------------
static void splitList(No *source, No **front, No **back);
static No* merge(No *a, No *b);
void mergeSort(No **headRef);

// ------------- Combate -------------
void rolagem(No *n);
void atacar(No* atacante_original, No* defensor_original); 
int lado_tem_vivos(No* lista); 
void remover_mortos(No **head);
No* montar_ordem(No* jogadores, No* inimigos);

// ------------- Fluxo de jogo -------------
void criar_personagem(No **lista);
void iniciar_combate(No **jogadores); 

// ---------------- Implementações ----------------
No* criar_no(char *nome, char *classe, int nivel, int dado) { 
    No* novo = malloc(sizeof(No));
    if (!novo) return NULL;

    // Alocação e cópia de strings (corrigido)
    novo->nome = strdup(nome);
    novo->classe = strdup(classe);
    if (!novo->nome || !novo->classe) {
        free_node(novo); // Libera o que foi alocado e o nó principal
        return NULL;
    }

    novo->nivel = nivel;
    novo->dado = dado;
    novo->iniciativa = 0; // Será rolada antes do combate
    novo->prox = NULL;

    calcular_atributos(novo);
    
    // A vida inicial é igual à constituição base calculada
    novo->vida_atual = novo->constituicao_base; 

    return novo;
}

void calcular_atributos(No* novo) {
    if (!novo) return;

    int nivel = novo->nivel;
    int dado = novo->dado > 0 ? novo->dado : 1; 

    // bases por classe simples
    int base_forca = 5, base_destreza = 5, base_const = 6, base_sab = 5;

    if (strcmp(novo->classe, "Guerreiro") == 0) { base_forca=8; base_destreza=5; base_const=8; base_sab=4; }
    else if (strcmp(novo->classe, "Barbaro") == 0) { base_forca=10; base_destreza=4; base_const=9; base_sab=3; }
    else if (strcmp(novo->classe, "Ladino") == 0) { base_forca=4; base_destreza=10; base_const=5; base_sab=4; }
    else if (strcmp(novo->classe, "Clerigo") == 0) { base_forca=5; base_destreza=4; base_const=7; base_sab=9; }
    else if (strcmp(novo->classe, "Mago") == 0) { base_forca=2; base_destreza=4; base_const=4; base_sab=10; }
    else if (strcmp(novo->classe, "Ranger") == 0) { base_forca=6; base_destreza=8; base_const=6; base_sab=5; }
    else if (strcmp(novo->classe, "Paladino") == 0) { base_forca=8; base_destreza=5; base_const=8; base_sab=6; }

    novo->forca = base_forca + (int)ceil(nivel * 1.2 + dado * 0.3);
    novo->destreza = base_destreza + (int)ceil(nivel * 0.8 + dado * 0.6);
    novo->constituicao_base = base_const + (int)ceil(nivel * 1.0 + dado * 0.4); 
    if (novo->constituicao_base < 1) novo->constituicao_base = 1;
    novo->sabedoria = base_sab + (int)ceil(nivel * 1.4 + dado * 0.2);
}

// Para a ordem de combate
No* copiar_no(No* p) {
    if (!p) return NULL;
    // cria uma cópia dos dados: usamos criar_no para alocar as strings
    No* novo = criar_no(p->nome, p->classe, p->nivel, p->dado); 
    if (!novo) return NULL;

    // Copia atributos já calculados e a iniciativa/vida_atual para o combate
    novo->forca = p->forca;
    novo->destreza = p->destreza;
    novo->constituicao_base = p->constituicao_base;
    novo->sabedoria = p->sabedoria;
    
    // **CRÍTICO:** Copia a vida atual do personagem real para a ordem de combate
    novo->vida_atual = p->vida_atual; 
    novo->iniciativa = p->iniciativa;

    return novo;
}

// Liberação de memória corrigida
void free_node(No *p) {
    if (!p) return;
    // Libera as strings alocadas com strdup/malloc em criar_no
    if (p->nome) free(p->nome); 
    if (p->classe) free(p->classe); 
    free(p);
}

void free_list(No **headRef) {
    if (!headRef || !*headRef) return;
    No *cur = *headRef;
    while (cur) {
        No *next = cur->prox;
        free_node(cur);
        cur = next;
    }
    *headRef = NULL;
}

// ------------ listas ----------
void inserir_no_comeco(No **lista, int classeId, char *nome) {
    if (!lista || !nome) return;

    char classe_buf[32];
    int dado = 6;
    switch (classeId) {
        case 1: dado = 10; strcpy(classe_buf, "Guerreiro"); break;
        case 2: dado = 12; strcpy(classe_buf, "Barbaro"); break;
        case 3: dado = 8;  strcpy(classe_buf, "Ladino"); break;
        case 4: dado = 6;  strcpy(classe_buf, "Clerigo"); break;
        case 5: dado = 6;  strcpy(classe_buf, "Mago"); break;
        case 6: dado = 10; strcpy(classe_buf, "Ranger"); break;
        case 7: dado = 10; strcpy(classe_buf, "Paladino"); break;
        default: strcpy(classe_buf, "Desconhecido"); dado = 6; break;
    }

    No* novo = criar_no(nome, classe_buf, 1, dado); 
    if (!novo) return;

    novo->prox = *lista;
    *lista = novo;
}

void inserir_no_fim(No** lista, No* novo) {
    if (!lista || !novo) return;
    novo->prox = NULL;
    if (*lista == NULL) {
        *lista = novo;
        return;
    }
    No* p = *lista;
    while (p->prox) p = p->prox;
    p->prox = novo;
}

void exibir_lista(No *p) {
    if (!p) { printf("Lista vazia.\n"); return; }
    while (p) {
        printf("\n-----------------\n");
        printf("Nome: %s\n", p->nome);
        printf("Classe: %s\n", p->classe);
        printf("Nivel: %d\n", p->nivel);
        printf("Dado: d%d\n", p->dado);
        printf("Forca: %d\n", p->forca);
        printf("Destreza: %d\n", p->destreza);
        printf("Constituicao (BASE): %d\n", p->constituicao_base); 
        printf("Vida Atual (HP): %d\n", p->vida_atual);
        printf("Sabedoria: %d\n", p->sabedoria);
        printf("Iniciativa: %d\n", p->iniciativa);
        p = p->prox;
    }
}

// -------------------------MergeSort 

static void splitList(No *source, No **front, No **back) {
    if (!source) { *front = *back = NULL; return; }
    No *slow = source;
    No *fast = source->prox;
    while (fast) {
        fast = fast->prox;
        if (fast) {
            slow = slow->prox;
            fast = fast->prox;
        }
    }
    *front = source;
    *back = slow->prox;
    slow->prox = NULL;
}

static No* merge(No *a, No *b) {
    if (!a) return b;
    if (!b) return a;
    No *result = NULL;
    if (a->iniciativa >= b->iniciativa) {
        result = a;
        result->prox = merge(a->prox, b);
    } else {
        result = b;
        result->prox = merge(a, b->prox);
    }
    return result;
}

void mergeSort(No **headRef) {
    if (!headRef || !*headRef) return;
    No *head = *headRef;
    if (!head->prox) return;
    No *a = NULL, *b = NULL;
    splitList(head, &a, &b);
    mergeSort(&a);
    mergeSort(&b);
    *headRef = merge(a, b);
}

// -------------------------Quick Sort

void troca_valores(No *a, No *b) {
    
    //Aqui troco os nomes e classes
    char *tmp_nome = a->nome; 
    a->nome = b->nome; 
    b->nome = tmp_nome;
    
    char *tmp_classe = a->classe; 
    a->classe = b->classe; 
    b->classe = tmp_classe;

    //Uso da macro SWAP para trocar as outras variáveis sem ter que repetir o código.
    int tmp;
    #define SWAP(x) tmp = a->x; a->x = b->x; b->x = tmp;

    SWAP(nivel)
    SWAP(dado)
    SWAP(iniciativa)
    SWAP(forca)
    SWAP(destreza)
    SWAP(constituicao_base)
    SWAP(sabedoria)
    SWAP(vida_atual)

    #undef SWAP
}

No* partition(No* low, No* high) {
    int pivot = high->iniciativa;
    No* i = low;
    No* j = low;

    while (j != high) {
        if (j->iniciativa >= pivot) {  // ordem decrescente
            troca_valores(i, j);
            i = i->prox;
        }
        j = j->prox;
    }
    troca_valores(i, high);
    return i;
}

void quickSortRec(No* low, No* high) {
    if (!low || !high || low == high) return;

    // Para achar nó anterior ao "high"
    No* prev = low;
    while (prev && prev->prox != high) prev = prev->prox;

    No* p = partition(low, high);

    if (p != low)
        quickSortRec(low, prev);

    if (p != high)
        quickSortRec(p->prox, high);
}

void quickSort(No **head) {
    if (!head || !*head) return;
    No* last = *head;
    while (last->prox) last = last->prox;
    quickSortRec(*head, last);
}


// ------------- Combate e utilitários -------------
void rolagem(No *n) {
    while (n) {
        if (n->dado > 0) n->iniciativa = (rand() % n->dado) + 1 + n->nivel;
        else n->iniciativa = 1;
        n = n->prox;
    }
}

void atacar(No* atacante_original, No* defensor_original) {
    if (!atacante_original || !defensor_original) return;
    if (atacante_original->vida_atual <= 0) return; 
    if (defensor_original->vida_atual <= 0) return; 

    printf("\n%s ataca %s\n", atacante_original->nome, defensor_original->nome);

    int rolagem_ataque = rand() % atacante_original->dado + 1;
    
    int defesa = defensor_original->destreza / 2 + 5; 
    printf("Rolagem: %d  Defesa alvo: %d (Destreza: %d)\n", rolagem_ataque, defesa, defensor_original->destreza);

    if (rolagem_ataque >= defesa) {
        int dano = atacante_original->forca;
        defensor_original->vida_atual -= dano;
        if (defensor_original->vida_atual < 0) defensor_original->vida_atual = 0;
        printf("Acertou! %s causa %d de dano. %s HP agora: %d\n", 
            atacante_original->nome, dano, defensor_original->nome, defensor_original->vida_atual);
        if (defensor_original->vida_atual == 0) {
            printf("💥 %s foi derrotado!\n", defensor_original->nome);
        }
    } else {
        printf("%s errou o ataque.\n", atacante_original->nome);
    }
}

int lado_tem_vivos(No* lista) {
    while (lista) {
        if (lista->vida_atual > 0) return 1;
        lista = lista->prox;
    }
    return 0;
}

// remove mortos e libera memória (altera a lista original)
void remover_mortos(No **head) {
    if (!head || !*head) return;
    No *cur = *head;
    No *prev = NULL;
    while (cur) {
        if (cur->vida_atual <= 0) { 
            No *tofree = cur;
            if (prev) {
                prev->prox = cur->prox;
                cur = prev->prox;
            } else {
                *head = cur->prox;
                cur = *head;
            }
            free_node(tofree);
        } else {
            prev = cur;
            cur = cur->prox;
        }
    }
}

// Função utilitária para encontrar o nó real (que tem o HP correto) a partir do nó da ordem
No* encontrar_no_real(No* lista_original, No* no_ordem) {
    No* p = lista_original;
    while (p) {
        // Compara pelo nome (chave única o suficiente para este exemplo)
        if (strcmp(p->nome, no_ordem->nome) == 0 && p->vida_atual > 0) {
            return p;
        }
        p = p->prox;
    }
    return NULL; // Não encontrado ou já morto
}

// monta Ordem de combate: copia todos os nós de jogadores e inimigos, retorna lista ordenada por iniciativa
No* montar_ordem(No* jogadores, No* inimigos) {
    No* ordem = NULL;
    No* p = jogadores;
    while (p) {
        if (p->vida_atual > 0) inserir_no_fim(&ordem, copiar_no(p));
        p = p->prox;
    }
    p = inimigos;
    while (p) {
        if (p->vida_atual > 0) inserir_no_fim(&ordem, copiar_no(p));
        p = p->prox;
    }
    quickSort(&ordem); // ordena por iniciativa (decrescente)
    return ordem;
}

// ----------------- Criar personagem---------------
void criar_personagem(No **lista) {
    if (!lista) return;
    char nome[64];
    int classe;
    printf("\n--- Criar Personagem ---\n");
    printf("Nome: ");
    do {
        if (!fgets(nome, sizeof(nome), stdin)) return;
        nome[strcspn(nome, "\n")] = '\0';
    } while (strlen(nome) == 0);

    printf("Classe:\n1-Guerreiro\n2-Barbaro\n3-Ladino\n4-Clerigo\n5-Mago\n6-Ranger\n7-Paladino\n");
    printf("Escolha (1-7): ");
    while (scanf("%d", &classe) != 1 || classe < 1 || classe > 7) {
        printf("Digite um numero valido (1-7): ");
        int c; while ((c = getchar()) != '\n' && c != EOF) {}
    }
    int c; while ((c = getchar()) != '\n' && c != EOF) {}

    inserir_no_comeco(lista, classe, nome);
    printf("✅ Personagem %s criado!\n", (*lista)->nome);
}

// ----------------- Iniciar combate -----------------
void iniciar_combate(No **jogadores) {
    if (!jogadores || !*jogadores) {
        printf("Voce precisa criar ao menos um personagem antes de enfrentar Keratz.\n");
        return;
    }

    // Cria inimigos (usando a função de criação corrigida)
    No *inimigos = NULL;
    inserir_no_fim(&inimigos, criar_no("Soldado de Keratz #1", "Guerreiro", 2, 8));
    inserir_no_fim(&inimigos, criar_no("Lacaio Espectral", "Mago", 3, 6));
    inserir_no_fim(&inimigos, criar_no("Espiao Rato", "Ladino", 2, 8));
    inserir_no_fim(&inimigos, criar_no("Elite de Keratz", "Paladino", 4, 10));
    inserir_no_fim(&inimigos, criar_no("Keratz, o Amaldiçoado", "Paladino", 8, 12));

    printf("\n--- Descricao ---\n");
    printf("O seu grupo encontra Keratz e seus capangas. Esta chegando a hora do confronto!\n");
    printf("Inimigos: ");
    No* temp = inimigos;
    while(temp) {
        printf("%s (HP: %d)%s", temp->nome, temp->vida_atual, temp->prox ? ", " : "");
        temp = temp->prox;
    }
    printf("\n");
    
    int round = 1;

    while (lado_tem_vivos(*jogadores) && lado_tem_vivos(inimigos)) {
        printf("\n====================\n");
        printf(" ⚔️  Ronda %d ⚔️ \n", round);
        printf("====================\n");

        rolagem(*jogadores);
        rolagem(inimigos);

        No* ordem = montar_ordem(*jogadores, inimigos);
        
        printf("\nOrdem de Ação:\n");
        No* p = ordem;
        while(p) {
            printf(" -> %s (Ini: %d, HP: %d)\n", p->nome, p->iniciativa, p->vida_atual);
            p = p->prox;
        }

        No* atual_na_ordem = ordem;
        while (atual_na_ordem) {
            
            if (atual_na_ordem->vida_atual > 0) { 
                
                // Encontra o nó original (com o HP real) do personagem que está agindo
                No* atacante_real = encontrar_no_real(*jogadores, atual_na_ordem);
                int ehJogador = (atacante_real != NULL);
                
                if (!ehJogador) {
                    atacante_real = encontrar_no_real(inimigos, atual_na_ordem);
                }
                
                if (atacante_real && atacante_real->vida_atual > 0) { // O atacante real está vivo
                    if (ehJogador) {
                        // Atacar primeiro inimigo vivo
                        No* alvo = inimigos;
                        while (alvo && alvo->vida_atual <= 0) alvo = alvo->prox;
                        if (alvo) atacar(atacante_real, alvo);
                    } else {
                        // É inimigo - atacar primeiro jogador vivo
                        No* alvo = *jogadores;
                        while (alvo && alvo->vida_atual <= 0) alvo = alvo->prox;
                        if (alvo) atacar(atacante_real, alvo);
                    }
                }
                
                // Checa se o combate terminou após o ataque (antes de continuar a ordem)
                if (!lado_tem_vivos(*jogadores) || !lado_tem_vivos(inimigos)) break;
            }
            atual_na_ordem = atual_na_ordem->prox;
        }

        // Remover mortos das listas reais e liberar memória
        remover_mortos(jogadores);
        remover_mortos(&inimigos);
        
        // Libera a lista de ordem da rodada
        free_list(&ordem); 
        round++;
    }

    printf("\n====================\n");
    if (!lado_tem_vivos(*jogadores)) {
        printf("💀 Todos os herois foram derrotados. Keratz venceu. 💀\n");
    } else {
        printf("🎉 Keratz e seus capangas foram derrotados! Voces venceram. 🎉\n");
    }
    printf("====================\n");

    // libera inimigos remanescentes
    free_list(&inimigos);
}

// ----------------- MAIN -----------------
int main() {
    // Para usar strdup
    #ifdef __linux__
    // strdup is POSIX.1-2001, generally available but compiler may warn.
    #else
    // Windows/MSVC: _strdup is available, or redefine strdup for compatibility
    #endif
    
    srand((unsigned)time(NULL));

    No *lista = NULL; // seus herois

    int escolha;
    printf("----------------------------------------------------------\n");
    printf("Objetivo: Monte uma equipe e enfrente Keratz e seus subordinados\n");

    do {
        printf("\nMenu:\n");
        printf("0 - Sair\n");
        printf("1 - Enfrentar (inicia o combate)\n");
        printf("2 - Adicionar personagem\n");
        printf("3 - Exibir equipe\n");
        printf("> ");

        if (scanf("%d", &escolha) != 1) {
            int cc; while ((cc = getchar()) != '\n' && cc != EOF) {}
            escolha = -1;
            continue;
        }
        int cc; while ((cc = getchar()) != '\n' && cc != EOF) {}

        switch (escolha) {
            case 0:
                free_list(&lista);
                printf("Saindo... ate a proxima.\n");
                return 0;

            case 1:
                if (!lista) {
                    printf("Crie ao menos um personagem primeiro.\n");
                } else {
                    iniciar_combate(&lista);
                }
                break;

            case 2:
                criar_personagem(&lista);
                break;

            case 3:
                printf("\nSua equipe:\n");
                exibir_lista(lista);
                break;

            default:
                printf("Opcao invalida.\n");
                break;
        }

    } while (1);

    return 0;
}
