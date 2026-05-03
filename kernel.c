#include "js/lexer.h"
#include "js/embedded.h"

/* --- KONSTANSOK ÉS VGA BEÁLLÍTÁSOK --- */
#define VGA_ADDR    0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define MAKE_COLOR(fg, bg) ((bg << 4) | fg)

#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define DARK_GREY 0x8
#define LIGHT_CYAN 0xB
#define WHITE 0xF
#define YELLOW 0xE
#define LIGHT_GREEN 0xA

static unsigned short* vga = (unsigned short*)VGA_ADDR;

/* --- VGA ÉS STRING SEGÉDFÜGGVÉNYEK --- */

void vga_put(int x, int y, unsigned char c, unsigned char color) {
    vga[y * VGA_WIDTH + x] = (color << 8) | c;
}

void clear_screen() {
    unsigned char col = MAKE_COLOR(WHITE, BLACK);
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = (col << 8) | ' ';
}

void vga_str(int x, int y, const char* s, unsigned char color) {
    // Sor törlése az írás helyén
    for (int i = 0; i < 60; i++) vga_put(x + i, y, ' ', color);
    for (int i = 0; s[i]; i++) vga_put(x + i, y, (unsigned char)s[i], color);
}

int v_strcmp(const char* a, const char* b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

void int_to_str(int n, char* s) {
    int i = 0;
    if (n == 0) { s[i++] = '0'; s[i] = '\0'; return; }
    while (n > 0) { s[i++] = (n % 10) + '0'; n /= 10; }
    s[i] = '\0';
    for (int j = 0; j < i / 2; j++) { char t = s[j]; s[j] = s[i-j-1]; s[i-j-1] = t; }
}

int str_to_int(const char* s) {
    int res = 0;
    for (int i = 0; s[i] >= '0' && s[i] <= '9'; i++) res = res * 10 + (s[i] - '0');
    return res;
}

/* --- BILLENTYŰZET DRIVER --- */

static inline unsigned char inb(unsigned short port) {
    unsigned char val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

unsigned char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

void get_input(char* buffer) {
    int i = 0;
    int start_x = 20; // Ahol a gépelés látszik
    while (1) {
        if (inb(0x64) & 1) {
            unsigned char scancode = inb(0x60);
            if (scancode < 128) {
                char c = scancode_to_ascii[scancode];
                if (c == '\n') {
                    buffer[i] = '\0';
                    return;
                } else if (c == '\b' && i > 0) {
                    i--;
                    vga_put(start_x + i, 15, ' ', MAKE_COLOR(WHITE, BLACK));
                } else if (c >= ' ' && i < 63) {
                    vga_put(start_x + i, 15, c, MAKE_COLOR(WHITE, BLACK));
                    buffer[i++] = c;
                }
            }
        }
    }
}

/* --- VÁLTOZÓ RENDSZER --- */

typedef struct {
    char name[32];
    int value;
    char s_value[64];
} Variable;

Variable var_table[30];
int var_count = 0;

void set_var_int(const char* name, int val) {
    for (int i = 0; i < var_count; i++) {
        if (v_strcmp(var_table[i].name, name) == 0) { var_table[i].value = val; return; }
    }
    int j = 0; while(name[j]) { var_table[var_count].name[j] = name[j]; j++; }
    var_table[var_count].name[j] = '\0';
    var_table[var_count].value = val;
    var_table[var_count].s_value[0] = '\0';
    var_count++;
}

int get_var(const char* name) {
    for (int i = 0; i < var_count; i++) {
        if (v_strcmp(var_table[i].name, name) == 0) return var_table[i].value;
    }
    return 0;
}

void set_var_str(const char* name, const char* val) {
    set_var_int(name, 0);
    int idx = var_count - 1;
    int j = 0; while(val[j]) { var_table[idx].s_value[j] = val[j]; j++; }
    var_table[idx].s_value[j] = '\0';
}

/* --- JS INTERPRETER (MOTOR) --- */

void execute_js(const char* source) {
    Lexer l;
    lexer_init(&l, source);
    Token t;
    int row = 17;

    while ((t = lexer_next(&l)).type != TOKEN_EOF) {
        // --- LET / VAR kezelés ---
        if (t.type == TOKEN_LET || t.type == TOKEN_VAR) {
            t = lexer_next(&l); // név
            char name[32]; int j = 0; while(t.value[j]) { name[j] = t.value[j]; j++; } name[j]='\0';
            lexer_next(&l); // =
            t = lexer_next(&l); 
            
            // PROMPT kezelése
            if (v_strcmp(t.value, "prompt") == 0) {
                lexer_next(&l); // (
                t = lexer_next(&l); // Kérdés szövege
                vga_str(2, 15, t.value, MAKE_COLOR(YELLOW, BLACK));
                lexer_next(&l); // )
                
                char input_buf[64];
                get_input(input_buf);
                set_var_str(name, input_buf);
                set_var_int(name, str_to_int(input_buf));
                vga_str(2, 15, "                                        ", BLACK);
            } else {
                // Szám vagy változó beolvasása
                int val = (t.type == TOKEN_NUMBER) ? str_to_int(t.value) : get_var(t.value);
                
                // Egyszerű matek (+)
                int pos_before = l.pos;
                t = lexer_next(&l);
                if (t.type == TOKEN_PLUS) {
                    t = lexer_next(&l);
                    val += (t.type == TOKEN_NUMBER) ? str_to_int(t.value) : get_var(t.value);
                } else {
                    l.pos = pos_before; // Visszatekerjük, ha nincs művelet
                }
                set_var_int(name, val);
            }
        }
        // --- CONSOLE.LOG kezelés ---
        else if (t.type == TOKEN_IDENT && v_strcmp(t.value, "console") == 0) {
            lexer_next(&l); lexer_next(&l); lexer_next(&l); // .log(
            char buf[80] = {0}; int p = 0;
            while ((t = lexer_next(&l)).type != TOKEN_RPAREN) {
                if (t.type == TOKEN_STRING) {
                    for(int k=0; t.value[k]; k++) buf[p++] = t.value[k];
                } else if (t.type == TOKEN_IDENT) {
                    int idx = -1;
                    for(int i=0; i<var_count; i++) if(v_strcmp(var_table[i].name, t.value)==0) idx=i;
                    if(idx != -1 && var_table[idx].s_value[0] != '\0') {
                        for(int k=0; var_table[idx].s_value[k]; k++) buf[p++] = var_table[idx].s_value[k];
                    } else {
                        char n_s[16]; int_to_str(get_var(t.value), n_s);
                        for(int k=0; n_s[k]; k++) buf[p++] = n_s[k];
                    }
                }
            }
            vga_str(2, row++, buf, MAKE_COLOR(LIGHT_GREEN, BLACK));
        }
    }
}

/* --- GRAFIKA ÉS BELÉPÉSI PONT --- */

void draw_logo() {
    unsigned char c1 = MAKE_COLOR(LIGHT_CYAN, BLACK);
    unsigned char c2 = MAKE_COLOR(CYAN, BLACK);
    unsigned char c3 = MAKE_COLOR(LIGHT_BLUE, BLACK);

    vga_str(1, 0,  "\xdb\xdb\x20\x20\x20\xdb\xdb", c1);
    vga_str(1, 1,  "\x20\xdb\xdb\x20\xdb\xdb\x20", c1);
    vga_str(1, 2,  "\x20\x20\xdb\xdb\xdb\x20\x20", c1);
    vga_str(1, 3,  "\x20\xdb\xdb\x20\xdb\xdb\x20", c1);
    vga_str(1, 4,  "\xdb\xdb\x20\x20\x20\xdb\xdb", c1);

    vga_str(10, 0, "\x20\xdb\xdb\xdb\xdb\x20", c2);
    vga_str(10, 1, "\xdb\xdb\x20\x20\xdb\xdb", c2);
    vga_str(10, 2, "\xdb\xdb\x20\x20\xdb\xdb", c2);
    vga_str(10, 3, "\xdb\xdb\x20\x20\xdb\xdb", c2);
    vga_str(10, 4, "\x20\xdb\xdb\xdb\xdb\x20", c2);

    vga_str(18, 0, "\x20\xdb\xdb\xdb\xdb\xdb", c3);
    vga_str(18, 1, "\xdb\xdb\x20\x20\x20\x20", c3);
    vga_str(18, 2, "\x20\xdb\xdb\xdb\xdb\x20", c3);
    vga_str(18, 3, "\x20\x20\x20\x20\xdb\xdb", c3);
    vga_str(18, 4, "\xdb\xdb\xdb\xdb\xdb\x20", c3);

    for (int i = 0; i < 80; i++) vga_put(i, 6, 0xCD, MAKE_COLOR(CYAN, BLACK));
    vga_str(2, 8, "Welcome to XOS - Interactive JS Mode", MAKE_COLOR(YELLOW, BLACK));
    vga_str(2, 9, "Status: System Live.", MAKE_COLOR(WHITE, BLACK));
}

void kernel_main() {
    clear_screen();
    draw_logo();
    vga_str(2, 14, "--- XOS JavaScript Output ---", MAKE_COLOR(CYAN, BLACK));
    
    execute_js(EMBEDDED_JS);
    
    while (1) {}
}