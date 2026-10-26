#include <gb/gb.h>
#include <stdio.h>
#include <stdlib.h>

#define TOTAL_COMMANDS 4
const char* cmd_list[TOTAL_COMMANDS] = {
    "IO_DIR",      // 1. Аппаратные регистры железа
    "SYS_VER",     // 2. Версия сборщика, аптайм и кнопки
    "MEMORY_INFO", // 3. Динамический замер свободной памяти
    "CLS"          // 4. Очистка экрана
};

UBYTE selected_cmd = 0; 
UBYTE last_pressed_button = 0; 

// --- ПРОТОТИПЫ ФУНКЦИЙ ---
unsigned int get_free_ram(void);
void clear_terminal(void);
void newline(void);
void draw_input_line(void);
void execute_command(void);
void main(void); 

// --- РЕАЛИЗАЦИЯ ФУНКЦИЙ ---

unsigned int get_free_ram(void) {
    unsigned int total_free = 0;
    unsigned int block_size = 32; 
    void* allocated_blocks[32]; // Задали фиксированный размер массива для стабильности памяти
    UBYTE block_count = 0;
    UBYTE i;

    while (block_count < 32) {
        void* ptr = malloc(block_size);
        if (ptr == NULL) {
            break; 
        }
        allocated_blocks[block_count] = ptr;
        block_count++;
        total_free += block_size;
    }

    for (i = 0; i < block_count; i++) {
        free(allocated_blocks[i]);
    }

    return total_free;
}

void clear_terminal(void) {
    UBYTE i;
    for(i = 0; i < 18; i++) {
        printf("\n");
    }
}

void newline(void) {
    printf("\n");
}

void draw_input_line(void) {
    printf("\rA:\\>%s_          ", cmd_list[selected_cmd]);
}

void execute_command(void) {
    newline(); 
    
    switch(selected_cmd) {
        case 0: // IO_DIR — Карта портов ввода-вывода Game Boy
            printf(">> HARDWARE REGISTERS:\n");
            printf(" LCDC_REG (SCRN): 0x%X\n", (unsigned int)LCDC_REG);
            printf(" NR52_REG (AUD):  0x%X\n", (unsigned int)NR52_REG);
            printf(" IE_REG   (INTR): 0x%X\n", (unsigned int)IE_REG);
            printf(" STAT_REG (LINE): 0x%X\n", (unsigned int)STAT_REG);
            break;
            
        case 1: // SYS_VER — Реальный аптайм и лог последнего нажатия
            printf(">> KERNEL & INPUT STATUS:\n");
            printf(" GBDK_BUILD_ID:  %d\n", __GBDK_VERSION);
            printf(" UPTIME TICKS:   %u\n", sys_time);
            printf(" LAST JOYPAD HEX: 0x%X\n", last_pressed_button);
            break;
            
        case 2: // MEMORY_INFO — Живой тест свободной кучи ОЗУ
            printf(">> CORE MEMORY MAP:\n");
            printf(" FREE HEAP: %u BYTES\n", get_free_ram());
            printf(" CMD_LIST   @ 0x%X\n", (unsigned int)&cmd_list);
            printf(" SEL_INDEX  @ 0x%X\n", (unsigned int)&selected_cmd);
            break;
            
        case 3: // CLS
            clear_terminal();
            break;
    }
    
    newline();
    draw_input_line();
}

void main(void) {
    UBYTE joydata;
    UBYTE last_joydata = 0;

    clear_terminal();
    printf("=== GB-DOS HARDWARE ===\n");
    printf("ALL REAL-TIME DATA ACTIVE\n\n");
    
    draw_input_line();

    while(1) {
        wait_vbl_done();
        joydata = joypad();

        if (joydata != last_joydata && joydata != 0) {
            
            last_pressed_button = joydata; 

            if (joydata & J_DOWN) {
                if (selected_cmd < TOTAL_COMMANDS - 1) selected_cmd++;
                else selected_cmd = 0; 
                draw_input_line();
            }
            else if (joydata & J_UP) {
                if (selected_cmd > 0) selected_cmd--;
                else selected_cmd = TOTAL_COMMANDS - 1; 
                draw_input_line();
            }
            else if ((joydata & J_A) || (joydata & J_START)) {
                execute_command();
            }
            
            delay(100); 
        }
        
        last_joydata = joydata;
    }
}
