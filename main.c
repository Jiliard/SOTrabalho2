#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define MAX_PROCESSES 10         // Número máximo de processos suportados
#define MAX_FRAMES 128           // Número máximo de quadros na memória física
#define FRAME_SIZE 16            // Tamanho de um quadro (em bytes)
#define MAX_PAGES_PER_PROCESS 8  // Número máximo de páginas por processo

typedef struct {
    int process_id;                 // Identificador do processo
    int page_count;                 // Número de páginas no processo
    int page_table[MAX_PAGES_PER_PROCESS];  // Tabela de páginas mapeando para quadros
} Process;

typedef struct {
    int memory_size;               // Tamanho total da memória física
    int num_frames;                // Número de quadros disponíveis
    bool frame_bitmap[MAX_FRAMES]; // Mapa de bits para os quadros livres
    float frame_usage[MAX_FRAMES]; // Percentual de ocupação por quadro
    int frame_process[MAX_FRAMES]; // ID do processo que ocupa cada quadro
    int frame_page[MAX_FRAMES];    // Página do processo que ocupa cada quadro
    unsigned char memory[MAX_FRAMES * FRAME_SIZE]; // Memória física representada por um vetor de bytes
} Memory;

void init_memory(Memory *mem, int memory_size, int frame_size) {
    mem->memory_size = memory_size;
    mem->num_frames = memory_size / frame_size;

    for (int i = 0; i < mem->num_frames; i++) {
        mem->frame_bitmap[i] = false;  // Marca todos os quadros como livres
        mem->frame_usage[i] = 0.0;     // Define ocupação inicial como 0%
        mem->frame_process[i] = -1;   // Nenhum processo associado
        mem->frame_page[i] = -1;      // Nenhuma página associada
    }

    for (int i = 0; i < mem->memory_size; i++) {
        mem->memory[i] = 0;            // Inicializa a memória com zeros
    }
}

void init_process(Process *process, int process_id, int process_size, int frame_size) {
    process->process_id = process_id;
    process->page_count = (process_size + frame_size - 1) / frame_size; // Arredondamento para cima

    for (int i = 0; i < MAX_PAGES_PER_PROCESS; i++) {
        process->page_table[i] = -1; // Inicializa a tabela de páginas com -1 (página não mapeada)
    }
}

bool allocate_memory(Memory *mem, Process *process, int process_size, int frame_size) {
    int allocated_frames = 0;
    int attempts = 0;

    while (allocated_frames < process->page_count && attempts < mem->num_frames * 2) {
        int random_frame = rand() % mem->num_frames; // Seleciona um quadro aleatório

        if (!mem->frame_bitmap[random_frame]) { // Verifica se o quadro está livre
            mem->frame_bitmap[random_frame] = true;
            process->page_table[allocated_frames] = random_frame; // Mapeia a página para o quadro

            if (allocated_frames == process->page_count - 1) { // Última página
                int remaining_bytes = process_size % frame_size;
                mem->frame_usage[random_frame] = remaining_bytes > 0 
                    ? (remaining_bytes / (float)frame_size) * 100.0
                    : 100.0; // Calcula ocupação se sobrar bytes
            } else {
                mem->frame_usage[random_frame] = 100.0; // Quadro totalmente ocupado
            }

            mem->frame_process[random_frame] = process->process_id; // Associa ao processo
            mem->frame_page[random_frame] = allocated_frames;       // Associa à página

            allocated_frames++;
        }
        attempts++;
    }

    return allocated_frames == process->page_count; // Retorna true se todas as páginas foram alocadas
}

void display_memory(Memory *mem) {
    int free_frames = 0;

    for (int i = 0; i < mem->num_frames; i++) {
        if (!mem->frame_bitmap[i]) {
            free_frames++;
        }
    }

    printf("\nMemória Livre: %.2f%%\n", (100.0 * free_frames) / mem->num_frames);

    for (int i = 0; i < mem->num_frames; i++) {
        if (mem->frame_bitmap[i]) {
            printf("Quadro %d: Ocupado, %.2f%% usado, Processo %d, Página %d\n",
                   i, mem->frame_usage[i], mem->frame_process[i], mem->frame_page[i]);
        } else {
            printf("Quadro %d: Livre\n", i);
        }
    }
}

void display_page_table(Process *process) {
    printf("Tabela de Páginas do Processo %d:\n", process->process_id);
    for (int i = 0; i < process->page_count; i++) {
        printf("Página %d -> Quadro %d\n", i, process->page_table[i]);
    }
}

int main() {
    Memory mem;
    Process processes[MAX_PROCESSES];
    int num_processes = 0;

    int memory_size, max_process_size, frame_size;

    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    printf("Digite o tamanho da memória física (em bytes): ");
    scanf("%d", &memory_size);
    printf("Digite o tamanho do quadro: ");
    scanf("%d", &frame_size);
    printf("Digite o tamanho máximo de um processo (em bytes): ");
    scanf("%d", &max_process_size);

    init_memory(&mem, memory_size, frame_size);

    while (1) {
        int option;
        printf("\n1. Visualizar memória\n2. Criar processo\n3. Visualizar tabela de páginas\n4. Sair\nEscolha uma opção: ");
        scanf("%d", &option);

        if (option == 1) {
            display_memory(&mem);
        } else if (option == 2) {
            int process_id, process_size;
            printf("Digite o ID do processo: ");
            scanf("%d", &process_id);
            printf("Digite o tamanho do processo (em bytes): ");
            scanf("%d", &process_size);

            if (process_size > max_process_size) {
                printf("Erro: Tamanho do processo excede o máximo permitido.\n");
                continue;
            }

            init_process(&processes[num_processes], process_id, process_size, frame_size);

            if (allocate_memory(&mem, &processes[num_processes], process_size, frame_size)) {
                printf("Processo %d criado com sucesso.\n", process_id);
                num_processes++;
            } else {
                printf("Erro: Memória insuficiente para alocar o processo.\n");
            }
        } else if (option == 3) {
            int process_id;
            printf("Digite o ID do processo: ");
            scanf("%d", &process_id);

            bool found = false;
            for (int i = 0; i < num_processes; i++) {
                if (processes[i].process_id == process_id) {
                    display_page_table(&processes[i]);
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("Erro: Processo não encontrado.\n");
            }
        } else if (option == 4) {
            break;
        }
    }

    return 0;
}
