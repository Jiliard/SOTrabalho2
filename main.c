#include <stdio.h>
#include <stdbool.h>

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
    unsigned char memory[MAX_FRAMES * FRAME_SIZE]; // Memória física representada por um vetor de bytes
} Memory;

void init_memory(Memory *mem, int memory_size) {
    mem->memory_size = memory_size;
    mem->num_frames = memory_size / FRAME_SIZE;

    for (int i = 0; i < mem->num_frames; i++) {
        mem->frame_bitmap[i] = false;  // Marca todos os quadros como livres
    }

    for (int i = 0; i < mem->memory_size; i++) {
        mem->memory[i] = 0;            // Inicializa a memória com zeros
    }
}

void init_process(Process *process, int process_id, int process_size) {
    process->process_id = process_id;
    process->page_count = (process_size + FRAME_SIZE - 1) / FRAME_SIZE;

    for (int i = 0; i < MAX_PAGES_PER_PROCESS; i++) {
        process->page_table[i] = -1;
    }
}

bool allocate_memory(Memory *mem, Process *process) {
    int allocated_frames = 0;

    for (int i = 0; i < mem->num_frames && allocated_frames < process->page_count; i++) {
        if (!mem->frame_bitmap[i]) {
            mem->frame_bitmap[i] = true;
            process->page_table[allocated_frames] = i;
            allocated_frames++;
        }
    }

    return allocated_frames == process->page_count;
}

void display_memory(Memory *mem) {
    int free_frames = 0;

    for (int i = 0; i < mem->num_frames; i++) {
        if (!mem->frame_bitmap[i]) {
            free_frames++;
        }
    }

    printf("Memória Livre: %.2f%%\n", (100.0 * free_frames) / mem->num_frames);

    for (int i = 0; i < mem->num_frames; i++) {
        printf("Quadro %d: %s\n", i, mem->frame_bitmap[i] ? "Ocupado" : "Livre");
    }
}

void display_page_table(Process *process) {
    printf("Tabela de Páginas do Processo %d:\n", process->process_id);
    for (int i = 0; i < process->page_count; i++) {
        printf("Página %d -> Quadro %d\n", i, process->page_table[i]);
    }
}

Process *find_process_by_id(Process processes[], int num_processes, int process_id) {
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].process_id == process_id) {
            return &processes[i];
        }
    }
    return NULL;
}

int main() {
    Memory mem;
    Process processes[MAX_PROCESSES];
    int num_processes = 0;

    int memory_size, max_process_size;
    printf("Digite o tamanho da memória física (em bytes): ");
    scanf("%d", &memory_size);
    printf("Digite o tamanho máximo de um processo (em bytes): ");
    scanf("%d", &max_process_size);

    init_memory(&mem, memory_size);

    while (1) {
        int option;
        printf("\n1. Visualizar memória\n2. Criar processo\n3. Visualizar tabela de páginas\n4. Sair\nEscolha uma opção: ");
        scanf("%d", &option);

        if (option == 1) {
            display_memory(&mem);
        } else if (option == 2) {
            if (num_processes >= MAX_PROCESSES) {
                printf("Erro: Número máximo de processos alcançado.\n");
                continue;
            }

            int process_id, process_size;
            printf("Digite o ID do processo: ");
            scanf("%d", &process_id);
            if (find_process_by_id(processes, num_processes, process_id)) {
                printf("Erro: Processo com ID %d já existe.\n", process_id);
                continue;
            }

            printf("Digite o tamanho do processo (em bytes): ");
            scanf("%d", &process_size);

            if (process_size > max_process_size) {
                printf("Erro: Tamanho do processo excede o máximo permitido.\n");
                continue;
            }

            Process *new_process = &processes[num_processes];
            init_process(new_process, process_id, process_size);

            if (allocate_memory(&mem, new_process)) {
                printf("Processo %d criado com sucesso.\n", process_id);
                num_processes++;
            } else {
                printf("Erro: Memória insuficiente para alocar o processo.\n");
            }
        } else if (option == 3) {
            int process_id;
            printf("Digite o ID do processo: ");
            scanf("%d", &process_id);

            Process *process = find_process_by_id(processes, num_processes, process_id);
            if (process) {
                display_page_table(process);
            } else {
                printf("Erro: Processo não encontrado.\n");
            }
        } else if (option == 4) {
            break;
        }
    }

    return 0;
}
