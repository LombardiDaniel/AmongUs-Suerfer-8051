#include<reg51.h>

sfr LCD_Port = 0x90; // Port 1
sbit rs = P1^0;
sbit rw = P1^1;
sbit en = P1^2;

sbit P1_1 = P1^3; // buttom

static unsigned int seed = 42; // Initial seed value
int RAND_MAX = 32767;

unsigned char table[2][17] = {"________________\0", "________________\0"};
unsigned char player_grid[2][2] = {"_\0", "_\0"};
unsigned char score_grid[4] = {'\0'};

code unsigned char player[8] = {0xE, 0x9, 0x17, 0x11, 0x11, 0x19, 0xA, 0x0};
code unsigned char tgt[8]   = {0xE, 0x1F, 0x11, 0x11, 0x1F, 0x15, 0xE, 0x0};
int playerX = 0;
int scoreX = 6;

code char playe	rId = 1;
code char tgtId  = 2;

unsigned int score = 0;


void sleep(unsigned int count) {
	int i, j;
	for (i = 0; i < count; i++)
		for(j=0; j < 112; j++);
}


void LCD_Command(char cmnd) {
    LCD_Port = (LCD_Port & 0x0f) | (cmnd & 0xf0); // snds upper nibble from command
    rs = 0;
    rw = 0;
    en = 1;
    sleep(1);
    en = 0;
    sleep(2);

    LCD_Port = (LCD_Port & 0x0f) | (cmnd << 4); // snds lower nibble from command
    en = 1;
    sleep(1);
    en = 0;
    sleep(2);
}

void LCD_Char(char data_byte) {
    // 10010000 = 0x90
    // 00001111

    LCD_Port = (LCD_Port & 0x0f) | (data_byte & 0xf0); // snds upper nibble from command
    rs = 1;
    rw = 0;
    en = 1;
    sleep(1);
    en = 0;
    sleep(2);

    LCD_Port = (LCD_Port & 0x0f) | (data_byte << 4); // snds lower nibble from command
    en = 1;
    sleep(1);
    en = 0;
    sleep(2);
}

void LCD_String(char* str) { // send str
    int i;
    for (i=0; str[i] != 0; i++) {
        LCD_Char(str[i]);
    }
}

void LCD_String_xy(char* str, char x, char y) {
    // first we set cursor
    if (y == 0)
        LCD_Command((x & 0x0f) | 0x80);
    else if (y == 1)
        LCD_Command((x & 0x0f) | 0xc0);
    
    LCD_String(str);
}

void LCD_init(void)
{
	sleep(20);				// Tempo de inicialização do LCD > 15ms
	LCD_Command(0x02); 		// Modo de 4 bits
	LCD_Command(0x28);		// Inicializa LCD para 2x16
	LCD_Command(0x0C);		// Liga o display e oculta o cursor
	LCD_Command(0x06);		// Habilita auto incromento do cursor
	LCD_Command(0x01);		// Limpa display
	LCD_Command(0x80);		// Coloca o cursor na posição inicial
}


void LCD_set_custom_char(unsigned char loc, unsigned char *char_mtrx) {
	// Seta o custom char
	int i;
	if (loc < 8) {
		/* Command 0x40 and onwards forces the device to point CGRAM address */
		LCD_Command(0x40 + (loc*8));
		
		for(i = 0; i < 8; i++)	/* Write 8 byte for generation of 1 character */
			LCD_Char(char_mtrx[i]);
  }   
}

// Function to generate a pseudo-random number between 0 and 32767 (inclusive)
int rand() {
    // LCG parameters (example constants, you can experiment with different values)
    const unsigned int a = 1664525;
    const unsigned int c = 1013904223;

    // Calculate the next pseudo-random value
    seed = (a * seed + c);

    // Return the lower 15 bits of the result (32767 is 2^15 - 1)
    return (int) (seed & RAND_MAX);
}

void shift_left_str(char *str) {
	int i = 0;

	for (i =0; i<16-1; i++) {
		str[i] = str[i+1];
	}

	str[16-1] = '_';
}

void render() {
	LCD_String_xy(table[0], 0, 0);
	LCD_String_xy(table[1], 0, 1);

	LCD_String_xy(player_grid[0], 0, 0);
	LCD_String_xy(player_grid[1], 0, 1);
}

void shiftTable() {
	shift_left_str(table[0]);
	shift_left_str(table[1]);
	// LCD_Command(0x18); // auto shift display
}

void itoa(int val) {
	score_grid[0] = '0' + val / 100;
	score_grid[1] = '0' + (val / 10) % 10;
	score_grid[2] = '0' + val % 10;
	score_grid[3] = '\0';
}

void game_loop() {

	int spawn = 0;
	int r = 0;
	int impact = 0;
	char wasted[17] = {"     WASTED     \0"};
	char clear[17] = {"                \0"};

	while (1) {
		// insert player
		player_grid[P1_1][playerX] = playerId;
		player_grid[!P1_1][playerX] = '_';
		impact = table[P1_1][playerX] == tgtId;

		if (impact) {
			// morreu
			LCD_String_xy(wasted, 0, 0);
			LCD_String_xy(clear, 0, 1);
			itoa(score); // already casts to score_grid
			LCD_String_xy(score_grid, scoreX, 1);
			break;
		}

		render();
		sleep(250 - (score >> 4));
		shiftTable();
		
		// Generating mobs
		r = rand();
		spawn = r < (RAND_MAX >> 3);
		if (spawn) {
			table[r & 0x1][15] = tgtId;
		}

		score++;
	}

	while(1);;

}


void main()
{	
	LCD_init();
	
	LCD_set_custom_char(playerId, player);
	LCD_set_custom_char(tgtId, tgt);

	game_loop();
}
