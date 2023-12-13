#include <nds.h>
#include <stdio.h>
#include <nf_lib.h>
#include <time.h>
#include <maxmod9.h>
#include "soundbank_bin.h"
#include "soundbank.h"

#define FRAMES_PER_ANIMATION 3

//Codigo para Mostrar Mensajes
#define LLAVE_OBTENIDA 0
#define ESPADA_OBTENIDA 1
#define ARMADURA_OBTENIDA 2
#define HISTORIA1 3
#define HISTORIA2 4
#define HISTORIA3 5
#define PUERTA 6
#define ATAQUE 7

//ID enemigos
#define SLIME_ID 9
#define THRONE_ID 10
#define LOSTSOUL_ID 11
#define LOSTSOUL2_ID 12


#define TIMER_SPEED (BUS_CLOCK/1024)

// -------------------------------------------------------------------------------
// Declaración de funciones
// -------------------------------------------------------------------------------


void playerControl(); //Mover el personaje cuando el status es el juego principal
void basicControl();   //Acceso a menus y movimiento en ellos
void playerAnimation(); //Anima el personaje y le da la perspectiva adecuada
void startScreen(); //Controles y mensajes del menú de inicio
void interruptionSetUp(); //Configura los Timers y el botón A despues de los combates
void QuickAnimation(); //Timer de animaciones rápidas
void colision(); //Colisiones, cambia los booleanos adecuados, permite conseguir cofres y empezar combates
void waitingPlayer(); //Turno del jugador durante el combate
void enemyTurn();  //Turno enemigo.
void touchMenu(); //Menú tactil
void zones();   //Permite cambiar de zona y altera los límites del mapa
void combatSetUp();  //Configura el combate
void showMessege();  //Muestra un mensaje  a partir de un digito
void Animation();  //Timer para animaciones
void unloadCombat(); //Carga el sprite de 64x64 para el combate
void ExtraAnimation();  //Timer para animaciones extra a otra velocidad
bool readingMessege();   //Espera a que el jugador lea
void ExtraControl();    //Botón A despues de un combate

typedef struct 
{
	float x, y;

	int id;
	int state;
	int anim_frame;
	int health;
	int max_health;
	int attack;
	int speed;
	int level;
} Objeto;

typedef struct
{
	float x, y;

	int id;
	int state;
	int anim_frame;
	int health;
	int max_health;
	int attack;
	int type;
	int speed;
	int level;
	bool alive;
} Enemigo;

enum SpriteState {W_DOWN = 1, W_RIGHT = 2, W_LEFT = 2,  W_UP = 0};

int screen_top = 0;
int screen_bottom = 196;
int screen_left = 0;
int screen_right = 256;

bool flip = false;
bool intro = true;
int contador_intro = 0;
int temporizador = 0;

//Estado del juego
bool out = false;
int status = 0;  // 0: pantalla de titulo / 1: menu principal / 2: juego principal / 3: combate / 4: visualizar mensajes / 5: pantalla de victoria
int status_anterior = 0;

//Selector del menu
int selector = 0;
bool start = false;

//Juego Principal
int zona = 0;
int touchMenuState = 0;
bool cargar = true;
bool interactuar = false;
bool inmovilizar = false;
bool cargar_zona = true;
bool combate = false;

//Movimiento
bool col_arriba = false;
bool col_abajo = false;
bool col_izquierda = false;
bool col_derecha = false;
int aleatorio2 = 0;

//Combate
bool player_turn = false;
int combat_selector = 0;
bool combat_option = false;
bool cargar_combate=true;
bool terminar_combate = false;
int aleatorio = 0;
bool animacion_ataque = false;
int contador_ataque = 0;
bool descargar = false;
bool enemy_attack = false;
bool protegido = false;
bool chargeAttack = false;
bool poder = false;

int word = 0; // 0 -->Alhy manipuladores;  1 -->Hjha asesinos ; 2 -->Meru  envidiosos
bool use_word = false;
bool show_soul = false;
int word_selector = 0;
int frame_sound = 0;

//Inventario
bool key = false;
bool sword = false;
bool armor = false;

//Objetos
Objeto sariel = { 128,96, .id = 0, .max_health = 20, .health = 20, .attack = 3, .speed = 3, .level=1 };
Objeto angelCore = { 96,0, .id = 1 };
Objeto enemy[6];

Enemigo slime = { 100,70, .id = SLIME_ID, .max_health = 30, .health = 15, .attack = 2, .speed = 2, .level = 2, .anim_frame = 0, .alive=true };
Enemigo lostSoul = { 100,70, .id = LOSTSOUL_ID, .max_health = 20, .health = 7, .attack = 2, .speed = 2, .level = 1, .anim_frame = 0, .alive = true };
Enemigo lostSoul2 = { 100,70, .id = LOSTSOUL2_ID, .max_health = 20, .health = 9, .attack = 4, .speed = 2, .level = 1, .anim_frame = 0, .alive = true };
Enemigo throne = { 100,70, .id = THRONE_ID, .max_health = 35, .health = 11, .attack = 10, .speed = 2, .level = 3, .anim_frame = 0, .alive = true };

Enemigo combat_enemy;  //Aquí almacenamos el enemigo al que se enfrenta el jugador

int anim_frame = 0;
bool movement = false;

int enemigo1_frames = 0;
int enemigo2_frames = 0;
int enemigo3_frames = 0;

//Mostrar Stats por pantalla
char salud[20];
char ataque[20];
char velocidad[20];
char nivel[20];

//Combate
char nivelEnemigo[30];
char saludEnemigo[30];
char saludJugador[30];
//Mensajes
char muerte[60];
char alma[120];

//-----------------------------------------------------------------
// Programa principal
//-----------------------------------------------------------------
int main(void) 
{

	

	//-----------------------------------------------------------------
	// Initialize the graphics engines
	//-----------------------------------------------------------------

	//Reiniciamos la semilla
	srand(time(NULL));
	
	NF_Set2D(0, 0);
	NF_Set2D(1, 0);

	// Inicializa los Sprites
	NF_InitSpriteBuffers();		// Inicializa los buffers para almacenar sprites y paletas
	NF_InitSpriteSys(0,128);		// Inicializa los sprites para la pantalla superior
	NF_InitTiledBgSys(1);		// Inicializa los sprites para la pantalla inferior

	// Inicializa los textos
	NF_InitTextSys(1); // texto pantalla inferior
	NF_InitTextSys(0); // texto pantalla superior

	// Inicializa los fondos tileados
	NF_InitTiledBgBuffers();	// Inicializa los buffers para almacenar fondos
	NF_InitTiledBgSys(0);		// Inicializa los fondos Tileados para la pantalla superior
	NF_InitTiledBgSys(1);		// Iniciliaza los fondos Tileados para la pantalla inferior

	// Define el ROOT e inicializa el sistema de archivos
	NF_SetRootFolder("NITROFS");	// Define la carpeta ROOT para usar NITROFS

	NF_LoadSpriteGfx("sprites/sarielSheet", sariel.id, 32, 32);
	NF_LoadSpritePal("sprites/sarielSheet", sariel.id);

	NF_LoadSpriteGfx("sprites/angelCore", angelCore.id, 64, 64);
	NF_LoadSpritePal("sprites/angelCore", angelCore.id);

	//screen, ram, slot
	NF_VramSpriteGfx(0, 0, 0, false);	// Sariel, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 0, 0);

	NF_VramSpriteGfx(0, 1, 1, false);	// angelCore, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 1, 1);

	//Ataque
	NF_LoadSpriteGfx("sprites/attackSheet", 3, 64, 64);
	NF_LoadSpritePal("sprites/attackSheet", 3);
	NF_VramSpriteGfx(0, 3, 3, false);	// angelCore, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 3, 3);

	//Cofre
	NF_LoadSpriteGfx("sprites/chest", 4, 32, 32);
	NF_LoadSpritePal("sprites/chest", 4);
	NF_VramSpriteGfx(0, 4, 4, false);	// angelCore, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 4, 4);
	NF_CreateSprite(0, 4, 4, 4, 10, 10);
	NF_ShowSprite(0, 4, false);

	//Enemigos Juego principal
	NF_LoadSpriteGfx("sprites/sarielM", 5, 32, 32);
	NF_LoadSpritePal("sprites/sarielM", 5);
	NF_VramSpriteGfx(0, 5, 5, false);	// angelCore, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 5, 5);
	NF_CreateSprite(0, 5, 5, 5, 20, 20);
	NF_ShowSprite(0, 5, false);

	NF_LoadSpriteGfx("sprites/lostSoul1", 6, 32, 32);
	NF_LoadSpritePal("sprites/lostSoul1", 6);
	NF_VramSpriteGfx(0, 6, 6, false);	// angelCore, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 6, 6);
	NF_CreateSprite(0, 6, 6, 6, 60, 60);
	NF_ShowSprite(0, 6, false); //Lost Soul

	NF_LoadSpriteGfx("sprites/slimeSheet32", 7, 32, 32);
	NF_LoadSpritePal("sprites/slimeSheet32", 7);
	NF_VramSpriteGfx(0, 7, 7, false);	// angelCore, copia todos los frames a la VRAM
	NF_VramSpritePal(0, 7, 7);
	NF_CreateSprite(0, 7, 7, 7, 50, 40);
	NF_ShowSprite(0, 7, false); //Lost Soul

	//NF_CreateSprite(Screen, ID, VRAM_Gfx_Slot, VRAM_Palette_Slot, X, Y);
	NF_CreateSprite(0, sariel.id, 0, 0, sariel.x, sariel.y);

	NF_CreateSprite(0, angelCore.id, 1, 1, angelCore.x, angelCore.y);

	//Fondos

	NF_LoadTiledBg("bg/titleDownScreen", "menu", 256, 256);
	NF_LoadTiledBg("bg/startScreen2", "caratula", 256, 256);
	NF_LoadTiledBg("bg/inGameTouchScreen", "touchMenu", 256, 256);
	NF_LoadTiledBg("bg/blackScreen", "black", 256, 256);

	NF_LoadTiledBg("bg/inventoryAll", "inventarioTodo", 256, 256);
	NF_LoadTiledBg("bg/inventoryArmor", "inventarioArmadura", 256, 256);
	NF_LoadTiledBg("bg/inventoryKey", "inventarioLlave", 256, 256);
	NF_LoadTiledBg("bg/inventorySK", "inventarioEspadaLlave", 256, 256);
	NF_LoadTiledBg("bg/inventoryAK", "inventarioArmaduraLlave", 256, 256);
	NF_LoadTiledBg("bg/inventory", "inventario", 256, 256);

	NF_LoadTiledBg("bg/zone1", "zonaInicial", 256, 256);
	NF_LoadTiledBg("bg/zone2", "zonaDerecha", 256, 256);
	NF_LoadTiledBg("bg/zone3", "zonaIzquierda", 256, 256);
	NF_LoadTiledBg("bg/zone4", "zonaFinal", 256, 256);
	NF_LoadTiledBg("bg/zone5", "zonaSubterranea", 256, 256);

	//Texto
	NF_LoadTextFont16("fonts/font16", "texto", 256, 256, 0);
	// screen, layer, rotation, name
	
	NF_CreateTextLayer16(1, 0, 0, "texto");
	NF_CreateTextLayer16(0, 0, 0, "texto");

	//Pantalla, capa, nº color, r, g ,b
	NF_DefineTextColor(1, 0, 0, 31, 31, 31); // Blanco --> 0
	NF_DefineTextColor(1, 0, 1, 15, 31, 15); // Verde clarito --> 1

	NF_DefineTextColor(0, 0, 0, 31, 31, 31); // Blanco --> 0

	//NF_WriteText(u8 screen, // Pantalla (0 – 1)
	//u8 layer, // Capa (0 – 3)
		//u8 x, // Posicion X
		//u8 y, // Posicion Y
		//const char* text // Texto a mostrar


	interruptionSetUp();

	//Mensajes Combate
	sprintf(muerte, "¿Fue tu juicio justo?");
	sprintf(saludJugador, "Player HP: %i ", sariel.health);

	//Musica MaxMod
	mmInitDefaultMem((mm_addr)soundbank_bin);
	mmLockChannels(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) | BIT(5) | BIT(6) | BIT(7));

	mmLoad(MOD_MUSIC);
	mmLoad(MOD_MAIN);
	mmLoad(MOD_COMBAT);

	mmStart(MOD_MUSIC, MM_PLAY_LOOP);
	mmSetModuleVolume(60);

	//Sonido
	soundEnable();
	NF_InitRawSoundBuffers();
	// File, ID, freq, format
	NF_LoadRawSound("sound/click", 0, 22050, 0);
	NF_LoadRawSound("sound/inventory", 1, 22050, 0);
	NF_LoadRawSound("sound/enemy", 2, 22050, 0);
	NF_LoadRawSound("sound/step", 3, 22050, 0);
	NF_LoadRawSound("sound/sword", 4, 22050, 0);
	NF_LoadRawSound("sound/bell", 5, 22050, 0);
	NF_LoadRawSound("sound/scare", 6, 22050, 0);

	NF_LoadRawSound("sound/crack", 7, 22050, 0);
	NF_LoadRawSound("sound/step2", 8, 22050, 0);
	
	//Posición Enemigos
	slime.x = 200;
	slime.y = 50;
	throne.x = 140;
	throne.y = 30;
	lostSoul2.x = 40;
	lostSoul2.y = 80;

	//-----------------------------------------------------------------
	// Bucle de juego
	//-----------------------------------------------------------------
	while(1) 
	{	
		switch (status) {
		case 0: //Pantalla de titulo

			NF_ShowSprite(0, sariel.id, false);
			scanKeys();
			int keys = keysHeld();
			
			
			NF_WriteText16(1, 0, 3, 2, "Pulsa Start para continuar");
			NF_UpdateTextLayers();

			if (keys & KEY_START)
			{
				status = 1;
				NF_ShowSprite(0, angelCore.id, false);
				NF_PlayRawSound(7, 60, 25, false, 0); // crack
			}

			break;

		case 1: //Menu de inicio
			intro = false;
			startScreen();
			basicControl();
			break;
		case 2: //Juego principal
			//NF_HideBg(1, 1);
			NF_ShowSprite(0, sariel.id, true);
			playerControl();
			playerAnimation();
			touchMenu();
			zones();
			colision();

			if (combate) {
				if (sariel.speed>combat_enemy.speed) {
					player_turn = true;
				}
				NF_ClearTextLayer16(1, 0);
				NF_UpdateTextLayers();
				cargar_combate = true;

				
				status = 3;
			}

			if (slime.alive==false && throne.alive == false && lostSoul2.alive == false) {
				NF_ShowSprite(0, sariel.id, false);

				NF_ClearTextLayer16(1, 0);
				status = 5;
				NF_UpdateTextLayers();
				NF_HideBg(0, 0);
				NF_HideBg(1, 1);
			}

			break;

		case 3: //Combate
			combatSetUp(combat_enemy);
			if (player_turn) {
				chargeAttack = false;
				poder = false;
				waitingPlayer();
			}
			else{
				enemyTurn();
			}
			break;

		case 4: //Visualizando Mensaje
			if (readingMessege()&&temporizador>1) {

				if (status_anterior==3) {
					status = 3;
					NF_CreateTiledBg(1, 1, "menu");
				}
				else {
					status = 2;
					NF_CreateTiledBg(1, 1, "touchMenu");
				}
				NF_ClearTextLayer16(1, 0);
				NF_UpdateTextLayers();
				cargar = true;
			}

			break;
		case 5:
			NF_WriteText16(1, 0, 3, 3, "Enemigos vencidos.");
			NF_WriteText16(1, 0, 3, 3, "Has impartido justicia.");
			NF_WriteText16(1, 0, 3, 4, "Sin embargo, podrias");
			NF_WriteText16(1, 0, 3, 5, "haberlo hecho mejor.");

			NF_WriteText16(1, 0, 3, 9, "Pulsa Start para salir.");
			NF_UpdateTextLayers();
			scanKeys();

			int keys2 = keysDown();
			if (keys2 & KEY_START)
			{
				out = true;
			}

		default:
			// code block
		}

		if (out==true) {
			break;
		}
		
		// Actualiza el array de OAM
		NF_SpriteOamSet(0);
		swiWaitForVBlank();	// Sincronismo vertical
		// Actualiza el OAM
		oamUpdate(&oamMain);
		//oamUpdate(&oamSub);
	}

	return 0;
}


void playerControl() {

	scanKeys();

	int keys = keysHeld();
	

	if (keys && !inmovilizar)
	{
		if (interactuar==true) {
			interactuar = false;
		}

		if ((keys & KEY_UP) && !col_arriba)
		{
			if (sariel.y >= screen_top) {
				sariel.y--;
				
			}
			frame_sound++;
			sariel.state = W_UP;
			movement = true;
			
		}

		if ((keys & KEY_LEFT) && !col_izquierda)
		{
			if (sariel.x >= screen_left) sariel.x--;
			else {
				if (zona == 0) {
					zona = 2;
					sariel.x = screen_right-10;
				}
				else if (zona == 1) {
					zona = 0;
					sariel.x = screen_right-10;
				}
			}
			frame_sound++;
			sariel.state = W_LEFT;
			flip = false;
			movement = true;
			
		}

		if ((keys & KEY_RIGHT) && !col_derecha)
		{
			if (sariel.x <= screen_right) sariel.x++;
			else {
				if (zona == 0) {
					zona = 1;
					sariel.x = screen_left+10;
				}
				else if (zona == 2) {
					zona = 0;
					sariel.x = screen_left+10;
				}
			}
			frame_sound++;
			sariel.state = W_RIGHT;
			flip = true;
			movement = true;
			
		}

		if ((keys & KEY_DOWN)&& !col_abajo)
		{
			if (sariel.y <= screen_bottom) sariel.y++;
			sariel.state = W_DOWN;
			movement = true;
			frame_sound++;
		}

	}
	else {
		movement = false;

		col_arriba = false;
		col_abajo = false;
		col_izquierda = false;
		col_derecha = false;

	}
	
	//Interactuar

	if (keys & KEY_A)
	{
		if (interactuar == true) {
			interactuar = false;
		}
		else {
			interactuar = true;
		}
	}

	if (frame_sound>22) {
		aleatorio2 = rand() % 2;
		if (aleatorio2 ==0) {
			NF_PlayRawSound(3, 127, 64, false, 0); // step
		}
		else {
			NF_PlayRawSound(8, 127, 64, false, 0); // step2
		}
		
		frame_sound = 0;
	}

	//Touch 

	
	touchPosition posicionXY;
	touchRead(&posicionXY);
	int keysT = keysDown();



	if (keysT & KEY_TOUCH) {

		cargar = true;

		if (touchMenuState == 0) {
			if (posicionXY.py < 96 && posicionXY.px < 128) {

				touchMenuState = 1;
				NF_PlayRawSound(1, 127, 64, false, 0); // inentory
			}
			else if (posicionXY.py < 96 && posicionXY.px > 128) {

				touchMenuState = 2;
				NF_PlayRawSound(0, 127, 64, false, 0); // click
			}
			else if (posicionXY.py > 96 && posicionXY.px < 128) {

				touchMenuState = 3;
				NF_PlayRawSound(0, 127, 64, false, 0); // click
			}
			else if (posicionXY.py > 96 && posicionXY.px > 128) {

				touchMenuState = 4;
				NF_PlayRawSound(0, 127, 64, false, 0); // click
			}
		}
		else {
			touchMenuState = 0;
		}
		
	}

}

void playerAnimation() {
	anim_frame++;
	if (anim_frame > 12) {
		anim_frame = 0;

		if (movement==true) {

			sariel.anim_frame++;
			if (sariel.anim_frame >= FRAMES_PER_ANIMATION) { sariel.anim_frame = 1; }
			// Salta al número de animación indicado
			if (sariel.state == W_RIGHT && flip == true) {
				NF_HflipSprite(0, 0, true);
				NF_SpriteFrame(0, sariel.id, sariel.anim_frame + FRAMES_PER_ANIMATION * sariel.state);

			}
			else {
				NF_HflipSprite(0, 0, false);
			}
			NF_SpriteFrame(0, sariel.id, sariel.anim_frame + FRAMES_PER_ANIMATION * sariel.state);

		}
		else {
			sariel.anim_frame = 0;
			NF_SpriteFrame(0, sariel.id, sariel.anim_frame + FRAMES_PER_ANIMATION * sariel.state);
			if (sariel.state == W_RIGHT && flip) {
				NF_HflipSprite(0, 0, true);
			}
		}
		


	}
	NF_MoveSprite(0, sariel.id, sariel.x, sariel.y);
}

void startScreen() {

	NF_ClearTextLayer16(1, 0);
	NF_CreateTiledBg(1, 1, "menu"); // Imagen en la pantalla inferior
	NF_CreateTiledBg(0, 0, "caratula"); // Imagen en la pantalla superior
	NF_ShowSprite(0, sariel.id, false);
	NF_ShowSprite(0, 4, false);
	NF_SetTextColor(1, 0, 0);
	NF_WriteText16(1, 0, 3, 9, "Por Alejandro Arevalo");
	//intro = true; //Empezar animación pantalla de inicio

	if (selector==0) {

		NF_SetTextColor(1, 0, 1);
		NF_WriteText16(1, 0, 3, 2, "Iniciar juego");
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 3, "Introduccion");
		NF_WriteText16(1, 0, 3, 4, "Controles");
		NF_WriteText16(1, 0, 3, 5, "Salir");
		NF_UpdateTextLayers();
		if (start) {
			status = 2;
			NF_DeleteTiledBg(0, 0);
			NF_DeleteTiledBg(1, 1);
			NF_ClearTextLayer16(1, 0);
			NF_UpdateTextLayers();
			NF_CreateTiledBg(0, 0, "zonaInicial"); // Imagen en la pantalla superior

			mmStart(MOD_MAIN, MM_PLAY_LOOP);
		}
	}
	else if (selector == 1) { //Introduccion
		if (!start) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 2, "Iniciar juego");
			NF_WriteText16(1, 0, 3, 4, "Controles");
			NF_WriteText16(1, 0, 3, 5, "Salir");
			NF_SetTextColor(1, 0, 1);
			NF_WriteText16(1, 0, 3, 3, "Introduccion");
			NF_UpdateTextLayers();
		}
		else {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 2, "Eres Sariel, angel de la");
			NF_WriteText16(1, 0, 3, 3, "justicia. Debes buscar las");
			NF_WriteText16(1, 0, 3, 4, "almas perdidas y juzgarlas");
			NF_WriteText16(1, 0, 3, 5, "de la forma adecuada.");
			NF_WriteText16(1, 0, 3, 6, "Algunos merecen tu palabra");
			NF_WriteText16(1, 0, 3, 7, "y perdon.");
			NF_WriteText16(1, 0, 3, 8, "Otros deben volver a morir.");
			NF_WriteText16(1, 0, 3, 8, "Intenta obrar con justicia.");
			NF_WriteText16(1, 0, 3, 10, "Pulsa A para continuar");
			NF_WriteText16(1, 0, 3, 9, "                     ");
			NF_UpdateTextLayers();
		}
	}
	else if (selector==2) { //Controles
		if (!start) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 2, "Iniciar juego");
			NF_WriteText16(1, 0, 3, 3, "Introduccion");
			NF_WriteText16(1, 0, 3, 5, "Salir");
			NF_SetTextColor(1, 0, 1);
			NF_WriteText16(1, 0, 3, 4, "Controles");
			NF_UpdateTextLayers();
		}
		else {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 2, "-A para interactuar.");
			NF_WriteText16(1, 0, 3, 3, "-Flechas para moverte.");
			NF_WriteText16(1, 0, 3, 4, "-En la pantalla inferior");
			NF_WriteText16(1, 0, 3, 5, "dispones del panel tactil.");
			NF_WriteText16(1, 0, 3, 6, "-Subir de nivel aviva tus");
			NF_WriteText16(1, 0, 3, 7, "stats. Solo podras subirlo");
			NF_WriteText16(1, 0, 3, 8, "luchando.");
			NF_WriteText16(1, 0, 3, 9, "                     ");
			NF_UpdateTextLayers();
		}
	}
	else if(selector==3) {
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 2, "Iniciar juego");
		NF_WriteText16(1, 0, 3, 3, "Introduccion");
		NF_WriteText16(1, 0, 3, 4, "Controles");
		NF_SetTextColor(1, 0, 1);
		NF_WriteText16(1, 0, 3, 5, "Salir");
		NF_UpdateTextLayers();
		if (start) {
			out = true;
		}
	}

}

void basicControl() {

	scanKeys();

	int keys = keysDown();

	if (keys)
	{
		if ((keys & KEY_UP)&&start==false)
		{
			if (selector == 0) {
				selector = 0;
			}
			else {
				selector--;
			}
			NF_PlayRawSound(0, 127, 64, false, 0); // click

		}
		if ((keys & KEY_DOWN) && start == false)
		{
			if (selector == 3) {
				selector = 0;
			}
			else {
				selector++;
			}
			NF_PlayRawSound(0, 127, 64, false, 0); // click
		}
		if (keys & KEY_A)
		{
			if (start) {
				start = false;
			}
			else {
				start = true;
			}
			NF_PlayRawSound(0, 127, 64, false, 0); // click
		}

	}

}


void interruptionSetUp() {

	irqEnable(IRQ_TIMER1); // Animaciones rápidas
	irqSet(IRQ_TIMER1, QuickAnimation);
	TIMER_DATA(1) = 0; 
	TIMER_CR(1) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;

	irqEnable(IRQ_TIMER2);
	irqSet(IRQ_TIMER2, Animation); // Animaciones lentas
	TIMER_DATA(2) = 32768; 
	TIMER_CR(2) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;

	irqEnable(IRQ_TIMER3);
	irqSet(IRQ_TIMER3, ExtraAnimation); 
	TIMER_DATA(3) = 50000; 
	TIMER_CR(3) = TIMER_DIV_1024 | TIMER_ENABLE | TIMER_IRQ_REQ;

	irqSet(IRQ_KEYS, ExtraControl);
	REG_KEYCNT = 0x4001; // A
}

void QuickAnimation() {

	if (temporizador<100) {
		temporizador++;
	}
	else {
		temporizador = 10;
	}

	

}

void Animation() {
	
	if (intro == true) {
		angelCore.anim_frame++;
		NF_SpriteFrame(0, angelCore.id, angelCore.anim_frame);
		angelCore.y += 15;
		NF_MoveSprite(0, angelCore.id, angelCore.x, angelCore.y);
		contador_intro++;
	}
	if (contador_intro > 5 && intro == true) {
		intro = false;
		NF_CreateTiledBg(0, 0, "caratula"); // Imagen en la pantalla superior

		NF_DeleteSprite(0, 1);
		NF_FreeSpriteGfx(0, 1);
		NF_UnloadSpritePal(1);
		NF_UnloadSpriteGfx(1);


	}

	if (enemigo1_frames < 2) { //throne
		enemigo1_frames++;
		NF_SpriteFrame(0, 5, enemigo1_frames);
		NF_MoveSprite(0, 5, throne.x, throne.y);
	}
	else {
		enemigo1_frames = 0;
		NF_SpriteFrame(0, 5, enemigo1_frames);
		NF_MoveSprite(0, 5, throne.x, throne.y);
	}

	if (enemigo2_frames < 5) { //lostsoul
		enemigo2_frames++;
		NF_SpriteFrame(0, 6, enemigo2_frames);
		NF_MoveSprite(0, 6, lostSoul2.x, lostSoul2.y);
	}
	else {
		enemigo2_frames = 0;
		NF_SpriteFrame(0, 6, enemigo2_frames);
		NF_MoveSprite(0, 6, lostSoul2.x, lostSoul2.y);
	}

	if (enemigo3_frames < 2) { //slime
		enemigo3_frames++;
		NF_SpriteFrame(0, 7, enemigo3_frames);
		NF_MoveSprite(0, 7, slime.x, slime.y);
	}
	else {
		enemigo3_frames = 0;
		NF_SpriteFrame(0, 7, enemigo3_frames);
		NF_MoveSprite(0, 7, slime.x, slime.y);
	}

	if (status == 3 && cargar_combate == false && combat_enemy.id==SLIME_ID) {
		if (combat_enemy.anim_frame < 2) {
			combat_enemy.anim_frame++;
			NF_SpriteFrame(0, SLIME_ID, combat_enemy.anim_frame); //combat_enemy.anim_frame
		}
		else {
			combat_enemy.anim_frame = 0;
			NF_SpriteFrame(0, SLIME_ID, combat_enemy.anim_frame);
		}
	}

	if (status == 3 && cargar_combate == false && combat_enemy.id == THRONE_ID) {
		if (combat_enemy.anim_frame < 2) {
			combat_enemy.anim_frame++;
			NF_SpriteFrame(0, THRONE_ID, combat_enemy.anim_frame); //combat_enemy.anim_frame
		}
		else {
			combat_enemy.anim_frame = 0;
			NF_SpriteFrame(0, THRONE_ID, combat_enemy.anim_frame);
		}
	}

	if (status == 3 && cargar_combate == false && combat_enemy.id == LOSTSOUL2_ID) {
		if (combat_enemy.anim_frame < 2) {
			combat_enemy.anim_frame++;
			NF_SpriteFrame(0, LOSTSOUL2_ID, combat_enemy.anim_frame); //combat_enemy.anim_frame
		}
		else {
			combat_enemy.anim_frame = 0;
			NF_SpriteFrame(0, LOSTSOUL2_ID, combat_enemy.anim_frame);
		}
	}

	if (slime.y < 199) {
		slime.y += 5;
	}
	else {
		throne.x -= 10;
	}
	if (slime.x > 150) {
		slime.x -= 4;
	}

	if (throne.x > 20) {
		throne.x -= 5;
	}
	else {
		throne.x += 10;
	}



}

void ExtraAnimation() {

	if (animacion_ataque) {
		if (contador_ataque == 0) {
			contador_ataque++;
			NF_CreateSprite(0, 3, 3, 3, 155, 80);
		}
		else if (contador_ataque == 1) {
			contador_ataque++;
			NF_SpriteFrame(0, 3, 1);
		}
		else {
			contador_ataque = 0;
			NF_SpriteFrame(0, 3, 0);
			NF_DeleteSprite(0, 3);
			animacion_ataque = false;
		}
	}

	


}

void colision() {

	switch (zona)
	{
	case 0: //Zona inicial
		
		if (sariel.x + 32 <= 200 + 32 && sariel.x + 32 >= 200 && sariel.y + 32 <= 50 + 32 && sariel.y + 32 >= 55) { //Cueva

			if (sariel.state == W_UP) {
				col_arriba = true;

				zona = 3;
				sariel.x = 128;
				sariel.y = 180;
			}

			if (sariel.state == W_DOWN)
			{
				col_abajo = true;
			}

			if (sariel.state == W_RIGHT && flip == true)
			{
				col_derecha = true;
			}

			if (sariel.state == W_LEFT && flip == false)
			{
				col_izquierda = true;
			}

			

		}
		else {
			col_arriba = false;
			col_abajo = false;
			col_izquierda = false;
			col_derecha = false;
		}

		break;
	case 1: //Zona derecha

		if (sariel.x + 32 <= 55 + 32 && sariel.x + 32 >= 55 && sariel.y + 32 <= 50 + 32 && sariel.y + 16 >= 50 && armor==false) { //Cofre

			if (sariel.state == W_UP) {
				col_arriba = true;
			}
				
			if (sariel.state == W_DOWN)
			{
				col_abajo = true;
			}

			if (sariel.state == W_RIGHT && flip==true)
			{
				col_derecha = true;
			}

			if (sariel.state == W_LEFT && flip == false)
			{
				col_izquierda = true;
			}

			if (sariel.state == W_UP && interactuar && armor==false) {

				armor = true;
				interactuar = false;
				showMessege(ARMADURA_OBTENIDA);
				NF_PlayRawSound(5, 100, 10, false, 0); // bell
			}

		}
		else {
			col_arriba = false;
			col_abajo = false;
			col_izquierda = false;
			col_derecha = false;
		}
		

		break;
	case 2: //Zona izquierda
				//limite derecha		//limite izquierda				//Limite inferior			//Limite Superior
		if (sariel.x + 50 <= 90 + 115 && sariel.x + 30 >= 105 && sariel.y + 32 <= 50 + 40 && sariel.y + 32 >= 45 ) { //Casa

			if (sariel.state == W_UP) {
				col_arriba = true;
			}

			if (sariel.state == W_DOWN)
			{
				col_abajo = true;
			}

			if (sariel.state == W_RIGHT && flip == true)
			{
				col_derecha = true;
			}

			if (sariel.state == W_LEFT && flip == false)
			{
				col_izquierda = true;
			}

		}
		else {
			col_arriba = false;
			col_abajo = false;
			col_izquierda = false;
			col_derecha = false;
		}
		
		NF_MoveSprite(0, 6, 40, 80);
		if (sariel.x + 32 <= lostSoul2.x+10 + 32 && sariel.x + 32 >= lostSoul2.x + 10 && sariel.y + 32 <= lostSoul2.y+5 + 32 && sariel.y + 16 >= lostSoul2.y + 10 && lostSoul2.alive==true) { //Enemigo LostSoul
			NF_PlayRawSound(6, 127, 20, false, 0); // scary
			combate = true;
			NF_ShowSprite(0, 6, false);
			combat_enemy = lostSoul2;
			sprintf(saludEnemigo, "Enemy HP: %i", combat_enemy.health);
			sprintf(nivelEnemigo, "Enemy Level: %i", combat_enemy.level);
		}

		break;
	case 3: //Zona subterranea
		
		if (sariel.x + 32 <= 25 + 32 && sariel.x + 32 >= 25 && sariel.y + 32 <= 60 + 32 && sariel.y + 32 >= 55 && key == false ) { //Cofre

			if (sariel.state == W_UP) {
				col_arriba = true;
				
			}

			if (sariel.state == W_DOWN)
			{
				col_abajo = true;
			}

			if (sariel.state == W_RIGHT && flip == true)
			{
				col_derecha = true;
			}

			if (sariel.state == W_LEFT && flip == false)
			{
				col_izquierda = true;
			}

			if (sariel.state == W_UP && interactuar && key == false) {

				key = true;
				interactuar = false;
				showMessege(LLAVE_OBTENIDA);
				NF_PlayRawSound(5, 100, 10, false, 0); // bell
			}
		}
		else {
			col_arriba = false;
			col_abajo = false;
			col_izquierda = false;
			col_derecha = false;
		}

		if (sariel.x + 32 <= slime.x+10 + 32 && sariel.x + 32 >= slime.x+10 && sariel.y + 32 <= slime.y+5 + 32 && sariel.y + 32 >= slime.y+10 && slime.alive == true) { //Enemigo Slime
			NF_PlayRawSound(6, 127, 20, false, 0); // scary
			combate = true;
			NF_ShowSprite(0, 7, false);
			combat_enemy = slime;
			sprintf(saludEnemigo, "Enemy HP: %i", combat_enemy.health);
			sprintf(nivelEnemigo, "Enemy Level: %i", combat_enemy.level);

		}

		break;
	case 4: //Zona final
		
		if (sariel.x + 32 <= 50 + 32 && sariel.x + 32 >= 50 && sariel.y + 32 <= 95 + 32 && sariel.y + 16 >= 85 && sword == false) { //Cofre

			if (sariel.state == W_UP) {
				col_arriba = true;

			}

			if (sariel.state == W_DOWN)
			{
				col_abajo = true;
			}

			if (sariel.state == W_RIGHT && flip == true)
			{
				col_derecha = true;
			}

			if (sariel.state == W_LEFT && flip == false)
			{
				col_izquierda = true;
			}
			if (sariel.state == W_UP && interactuar && sword == false) {

				sword = true;
				sariel.attack += 3;
				interactuar = false;
				showMessege(1); // Espada_obtenida
				NF_PlayRawSound(5, 100, 10, false, 0); // bell
			}

		}
		else {
			col_arriba = false;
			col_abajo = false;
			col_izquierda = false;
			col_derecha = false;
		}
		

		if (sariel.x + 32 <= throne.x+10 + 32 && sariel.x + 32 >= throne.x+10 && sariel.y + 32 <= throne.y+5 + 32 && sariel.y + 16 >= throne.y+10 && throne.alive == true) { //Enemigo Angel
			NF_PlayRawSound(6, 127, 20, false, 0); // scary
			combate = true;
			NF_ShowSprite(0, 5, false);
			combat_enemy = throne;
			sprintf(saludEnemigo, "Enemy HP: %i", combat_enemy.health);
			sprintf(nivelEnemigo, "Enemy Level: %i", combat_enemy.level);

		}

		break;
	default:
		break;
	}

}

void waitingPlayer() {

	scanKeys();

	int keys = keysDown();

	NF_CreateTiledBg(1, 1, "black");
	NF_CreateTiledBg(0, 1, "black");

	NF_SetTextColor(1, 0, 0);
	NF_WriteText16(1, 0, 1, 10, saludEnemigo);
	NF_WriteText16(1, 0, 16, 10, nivelEnemigo);

	NF_WriteText16(1, 0, 1, 1, saludJugador);
	NF_UpdateTextLayers();

	enemy_attack = true;


	if (combat_enemy.health<=0) {
		NF_ClearTextLayer16(1, 0);
		NF_WriteText16(1, 0, 4, 4, muerte);
		NF_UpdateTextLayers();

		irqEnable(IRQ_KEYS);

		if (terminar_combate == true) {
			NF_PlayRawSound(0, 127, 64, false, 0); // click

			if (combat_enemy.id == SLIME_ID) {
				slime.alive = false;
			}
			else if (combat_enemy.id == THRONE_ID) {
				throne.alive = false;
			}
			else {
				lostSoul2.alive = false;
			}

			irqDisable(IRQ_KEYS);
			status = 2;
			descargar = true;
			unloadCombat();
			terminar_combate = false;

		}
	}

	if (sariel.health <= 0) {
		out = true;
	}

	if (keys)
	{
		if (keys & KEY_UP)
		{
			if (combat_selector > 0 && combat_selector!=4 && combat_selector != 5 && !combat_option) {
				combat_selector--;
			}

			if (word_selector > 0) {
				word_selector--;
			}
			NF_PlayRawSound(0, 127, 64, false, 0); // click
		}
		if (keys & KEY_DOWN)
		{
			if (combat_selector != 4 && combat_selector != 5 && !combat_option) {
				
				if (combat_selector < 2 && armor == false) {
					combat_selector++;
				}
				if (combat_selector < 3 && armor == true) {
					combat_selector++;
				}
				
			}

			if (word_selector < 2) {
				word_selector++;
			}

			NF_PlayRawSound(0, 127, 64, false, 0); // click
		}
		if (keys & KEY_A)
		{
			if (combat_option == false && terminar_combate !=true) {
				combat_option = true;
			}
			else {
				if (combat_selector==4) {
					use_word = true;
				}
				
				show_soul = true;
			}

			if (combat_enemy.alive==false) {
				terminar_combate = true;
			}
			
			
		}
	}

	

	switch (combat_selector)
	{
	case 0:
		
		NF_SetTextColor(1, 0, 1);
		if (sword) {
			NF_WriteText16(1, 0, 3, 2, "Rebanar con la espada");
		}
		else {
			NF_WriteText16(1, 0, 3, 2, "Atacar");
		}
		
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 4, "Leer alma");
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 6, "Usar Palabra");
		if (armor) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 8, "Protegerse");
		}
		NF_UpdateTextLayers();
		if (combat_option&& animacion_ataque ==false ) {
			NF_PlayRawSound(4, 127, 20, false, 0); // sword
			combat_enemy.health = combat_enemy.health - sariel.attack;
			animacion_ataque=true;
			combat_option = false;
			descargar = true;
			sprintf(saludEnemigo, "Enemy HP: %i ", combat_enemy.health);
			NF_ClearTextLayer16(1, 0);
			aleatorio = rand() % 8;

			player_turn = false;
			temporizador = 0;
		}
		break;
	case 1:
		NF_SetTextColor(1, 0, 0);
		if (sword) {
			NF_WriteText16(1, 0, 3, 2, "Rebanar con la espada");
		}
		else {
			NF_WriteText16(1, 0, 3, 2, "Atacar");
		}
		NF_SetTextColor(1, 0, 1);
		NF_WriteText16(1, 0, 3, 4, "Leer alma");
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 6, "Usar Palabra");
		if (armor) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 8, "Protegerse");
		}
		NF_UpdateTextLayers();
		if (combat_option) {
			NF_PlayRawSound(0, 127, 64, false, 0); // click
			combat_selector = 5;
			NF_ClearTextLayer16(1, 0);
			show_soul = false;
			

		}
		break;
	case 2:
		NF_SetTextColor(1, 0, 0);
		if (sword) {
			NF_WriteText16(1, 0, 3, 2, "Rebanar con la espada");
		}
		else {
			NF_WriteText16(1, 0, 3, 2, "Atacar");
		}
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 4, "Leer alma");
		NF_SetTextColor(1, 0, 1);
		NF_WriteText16(1, 0, 3, 6, "Usar Palabra");
		if (armor) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 8, "Protegerse");
		}
		NF_UpdateTextLayers();
		if (combat_option) {
			NF_PlayRawSound(0, 127, 64, false, 0); // click
			combat_selector = 4;
			NF_ClearTextLayer16(1, 0);
			combat_option = false;
			word_selector = 0;
		}
		break;
	case 3:
		NF_SetTextColor(1, 0, 0);
		if (sword) {
			NF_WriteText16(1, 0, 3, 2, "Rebanar con la espada");
		}
		else {
			NF_WriteText16(1, 0, 3, 2, "Atacar");
		}
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 4, "Leer alma");
		NF_SetTextColor(1, 0, 0);
		NF_WriteText16(1, 0, 3, 6, "Usar Palabra");
		if (armor) {
			NF_SetTextColor(1, 0, 1);
			NF_WriteText16(1, 0, 3, 8, "Protegerse");
		}
		NF_UpdateTextLayers();
		if (combat_option) {
			NF_PlayRawSound(0, 127, 64, false, 0); // click
			combat_selector = 6;
			NF_ClearTextLayer16(1, 0);
			temporizador = 0;
			combat_option = false;
		}
		break;
	case 4:

		if (word_selector==0) {
			NF_SetTextColor(1, 0, 1);
			NF_WriteText16(1, 0, 3, 2, "Alhy");
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 4, "Hjha");
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 6, "Meru");
			NF_UpdateTextLayers();
			if (use_word) {
				NF_PlayRawSound(0, 127, 64, false, 0); // click
				word = 0;
				player_turn = false;
				aleatorio = rand() % 8;
				NF_ClearTextLayer16(1, 0);
				temporizador = 0;
			}
		}
		else if (word_selector==1) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 2, "Alhy");
			NF_SetTextColor(1, 0, 1);
			NF_WriteText16(1, 0, 3, 4, "Hjha");
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 6, "Meru");
			NF_UpdateTextLayers();
			if (use_word) {
				NF_PlayRawSound(0, 127, 64, false, 0); // click
				word = 1;
				player_turn = false;
				aleatorio = rand() % 8;
				NF_ClearTextLayer16(1, 0);
				combat_option = false;
				temporizador = 0;
			}
		}
		else if(word_selector == 2) {
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 2, "Alhy");
			NF_SetTextColor(1, 0, 0);
			NF_WriteText16(1, 0, 3, 4, "Hjha");
			NF_SetTextColor(1, 0, 1);
			NF_WriteText16(1, 0, 3, 6, "Meru");
			NF_UpdateTextLayers();
			if (use_word) {
				NF_PlayRawSound(0, 127, 64, false, 0); // click
				word = 2;
				player_turn = false;
				aleatorio = rand() % 8;
				NF_ClearTextLayer16(1, 0);
				combat_option = false;
				temporizador = 0;
			}
		}
		break;

	case 5:

		if (combat_enemy.id== SLIME_ID) {
			NF_WriteText16(1, 0, 2, 2, "He odiado, he delinquido");
			NF_WriteText16(1, 0, 2, 3, "use la violencia en los");
			NF_WriteText16(1, 0, 2, 4, "inocentes. Han fallecido");
			NF_WriteText16(1, 0, 2, 5, "bajo mis manos.");
			NF_UpdateTextLayers();
		}
		else if (combat_enemy.id == THRONE_ID) {
			NF_WriteText16(1, 0, 2, 2, "Senti deseo de ser otros,");
			NF_WriteText16(1, 0, 2, 3, "usurpe aquello que les");
			NF_WriteText16(1, 0, 2, 4, "pertenecia y ahora nada queda.");
			NF_UpdateTextLayers();
		}
		else {
			NF_WriteText16(1, 0, 2, 2, "Estoy perdido. Nada veo,");
			NF_WriteText16(1, 0, 2, 3, "nada entiendo. Solo puedo");
			NF_WriteText16(1, 0, 2, 4, "intentar correr.");
			NF_UpdateTextLayers();
		}
		
		if (show_soul) {
			NF_PlayRawSound(0, 127, 64, false, 0); // click
			player_turn = false;
			aleatorio = rand() % 8;
			temporizador = 0;
			show_soul = false;
			NF_ClearTextLayer16(1, 0);
			combat_option = false;
		}
		
		break;
	case 6: //Protegerse
		NF_WriteText16(1, 0, 2, 2, "Preparas tu armadura.");
		NF_WriteText16(1, 0, 2, 3, "Te protegeras ante");
		NF_WriteText16(1, 0, 2, 4, "el proximo ataque.");
		NF_UpdateTextLayers();

		if (temporizador>1) {
			player_turn = false;
			aleatorio = rand() % 8;
			temporizador = 0;
			show_soul = false;
			NF_ClearTextLayer16(1, 0);
			combat_option = false;
			protegido = true;
		}
		break;

	case 7:  //Muerte del enemigo

		break;
	default:

		break;
	}

	
}


void enemyTurn() {
	//scanKeys();
	//int keys = keysDown();
	combat_selector = 0;
	word_selector = 0;
	combat_option = false;

	if (use_word) { //Palabra usada

		if (word==0) {
			if (combat_enemy.id == THRONE_ID) {
				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo Ha Sido Perdonado");
				NF_UpdateTextLayers();

				if (temporizador>1) {
					combat_enemy.health = 0;
					combat_enemy.alive = false;
					use_word = false;
					
				}

			}
			else {
				combat_enemy.attack += 1;
				use_word = false;
			}
			
		}
		else if (word==1) {
			
			if (combat_enemy.id == SLIME_ID) {
				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo Ha Sido Perdonado");
				NF_UpdateTextLayers();

				if (temporizador > 1) {
					combat_enemy.health = 0;
					combat_enemy.alive = false;
					use_word = false;
				}
			}
			else {
				combat_enemy.attack += 1;
				use_word = false;
			}
			
			
		}
		else
		{
			if(combat_enemy.id == LOSTSOUL2_ID) {
				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo Ha Sido Perdonado");
				NF_UpdateTextLayers();
				combat_enemy.alive = false;

				if (temporizador > 1) {
					combat_enemy.health = 0;
					use_word = false;
				}
			}
			else {
				combat_enemy.attack += 1;
				use_word = false;
			}

		}
	}

	if (combat_enemy.health <= 0 && animacion_ataque == false) {
		NF_ClearTextLayer16(1, 0);
		NF_WriteText16(1, 0, 4, 2, "Enemigo Ha Sido Vencido");

		if (use_word==false) {
			NF_WriteText16(1, 0, 4, 6, "Subes de nivel!");
		}
		
		NF_WriteText16(1, 0, 5, 4, muerte);
		NF_UpdateTextLayers();
		if (temporizador>1) {
			irqEnable(IRQ_KEYS);
		}
		combat_enemy.alive = false;

		if (terminar_combate == true) {

			if (combat_enemy.id == SLIME_ID) {
				slime.alive = false;
			}
			else if (combat_enemy.id == THRONE_ID) {
				throne.alive = false;
			}
			else {
				lostSoul2.alive = false;
			}

			if (use_word == false) {
				sariel.level++;
				sariel.attack += 2;
				sariel.speed += 1;
				sariel.health += 0;
			}

			NF_ClearTextLayer16(1, 0);
			NF_UpdateTextLayers();
			irqDisable(IRQ_KEYS);
			status = 2;
			descargar = true;
			unloadCombat();
			terminar_combate = false;
			combate = false;
			cargar = true;
		}

	}

	if(use_word==false && combat_enemy.alive==true)
	{
		
		if (combat_enemy.id == SLIME_ID) {

			if (aleatorio < 5 && animacion_ataque == false && combat_enemy.alive == true && poder == false) {
				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo Acumula Poder");
				combat_enemy.health += 2;
				combat_enemy.attack += 2;
				NF_UpdateTextLayers();
				poder = true;
			}
			else if (aleatorio > 5 && animacion_ataque == false && combat_enemy.alive == true) {

				if (use_word == false && enemy_attack == true) {
					NF_SetTextColor(1, 0, 0);
					NF_WriteText16(1, 0, 3, 2, "Enemigo Ataca");
					if (protegido == false) {
						sariel.health = sariel.health - combat_enemy.attack;
					}
					sprintf(saludJugador, "Player HP: %i ", sariel.health);
					enemy_attack = false;
					NF_UpdateTextLayers();
					NF_PlayRawSound(2, 127, 64, false, 0); // click
					protegido = false;
				}
			}

		}
		else if (combat_enemy.id == THRONE_ID) {
			if (chargeAttack && aleatorio > 4 ) {
				

				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo Ataca con");
				NF_WriteText16(1, 0, 3, 3, "todas sus fuerzas!");
				if (protegido == false) {
					sariel.health = sariel.health - combat_enemy.attack-10;
				}
				sprintf(saludJugador, "Player HP: %i ", sariel.health);
				enemy_attack = false;
				NF_UpdateTextLayers();
				NF_PlayRawSound(2, 127, 64, false, 0); // click
				protegido = false;

			}
			if (aleatorio < 4 && animacion_ataque == false && combat_enemy.alive == true && chargeAttack==false) {
				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo carga ataque");
				NF_WriteText16(1, 0, 3, 3, "ultrapoderoso.");
				NF_WriteText16(1, 0, 3, 4, "Ten cuidado!");
				chargeAttack = true;
				NF_UpdateTextLayers();
			}
			else if (aleatorio>5 && animacion_ataque == false && combat_enemy.alive == true && chargeAttack==false) {

				if (use_word == false && enemy_attack == true) {
					NF_SetTextColor(1, 0, 0);
					NF_WriteText16(1, 0, 3, 2, "Enemigo Ataca");
					if (protegido == false) {
						sariel.health = sariel.health - combat_enemy.attack;
					}
					sprintf(saludJugador, "Player HP: %i ", sariel.health);
					enemy_attack = false;
					NF_UpdateTextLayers();
					NF_PlayRawSound(2, 127, 64, false, 0); // click
					protegido = false;
				}
			}

		}
		else {
			if (aleatorio < 3 && animacion_ataque == false && combat_enemy.alive == true) {
				NF_SetTextColor(1, 0, 0);
				NF_WriteText16(1, 0, 3, 2, "Enemigo Prepara Tecnica");
				NF_UpdateTextLayers();
			}
			else if (aleatorio>3 && animacion_ataque == false && combat_enemy.alive == true) {

				if (use_word == false && enemy_attack == true) {
					NF_SetTextColor(1, 0, 0);
					NF_WriteText16(1, 0, 3, 2, "Enemigo Ataca");
					if (protegido == false) {
						sariel.health = sariel.health - combat_enemy.attack;
					}
					sprintf(saludJugador, "Player HP: %i ", sariel.health);
					enemy_attack = false;
					NF_UpdateTextLayers();
					NF_PlayRawSound(2, 127, 64, false, 0); // click
					protegido = false;
				}
			}
		}
		
	}


	
	if (temporizador > 1)
	{
	
		NF_ClearTextLayer16(1, 0);
		player_turn = true;
		
	}
	
}

void touchMenu() {

	switch (touchMenuState)
	{
	case 0:
		if (cargar) {
			NF_ClearTextLayer16(1, 0);
			NF_CreateTiledBg(1, 1, "touchMenu");
			NF_WriteText16(1, 0, 4, 3, "Inventario");
			NF_WriteText16(1, 0, 21, 3, "Stats");
			NF_WriteText16(1, 0, 5, 8, "Palabras");
			NF_WriteText16(1, 0, 19, 8, "Salir");
			NF_UpdateTextLayers();
		}
		cargar = false;
		break;
	case 1:
		if (cargar) {
			NF_ClearTextLayer16(1, 0);
			NF_UpdateTextLayers();
			
			if (key && armor && sword) {
				NF_CreateTiledBg(1, 1, "inventarioTodo");
			}
			else if (key && !armor && !sword) {
				NF_CreateTiledBg(1, 1, "inventarioLlave");
			}
			else if (key&&armor&&!sword) {
				NF_CreateTiledBg(1, 1, "inventarioArmaduraLlave");
			}
			else if (key&&sword&&!armor){
				NF_CreateTiledBg(1, 1, "inventarioEspadaLlave");
			}
			else if (armor&&!sword&&!key) {
				NF_CreateTiledBg(1, 1, "inventarioArmadura");
			}
			else {
				NF_CreateTiledBg(1, 1, "inventario");
			}

		}
		cargar = false;
		

		break;
	case 2:
		if (cargar) {
			NF_CreateTiledBg(1, 1, "menu");
			NF_ClearTextLayer16(1, 0);
			

			sprintf(salud, " %i HP", sariel.health);
			sprintf(ataque, " %i Attack Points", sariel.attack);
			sprintf(velocidad, " %i Speed Points", sariel.speed);
			sprintf(nivel, " %i lvl", sariel.level);

			NF_WriteText16(1, 0, 4, 4, salud);
			NF_WriteText16(1, 0, 4, 5, ataque);
			NF_WriteText16(1, 0, 4, 6, velocidad);
			NF_WriteText16(1, 0, 4, 7, nivel);
			NF_UpdateTextLayers();
		}
		cargar = false;
		break;
	case 3:
		if (cargar) {
			NF_CreateTiledBg(1, 1, "menu");
			NF_ClearTextLayer16(1, 0);
			NF_WriteText16(1, 0, 3, 2, "Alhy. Perdona a los");
			NF_WriteText16(1, 0, 3, 3, "manipuladores.");
			NF_WriteText16(1, 0, 3, 5, "Hjha. Tiene piedad");
			NF_WriteText16(1, 0, 3, 6, "con los asesinos.");
			NF_WriteText16(1, 0, 3, 8, "Meru. Libera a los");
			NF_WriteText16(1, 0, 3, 9, "envidiosos y resentidos.");
			NF_UpdateTextLayers();
		}
		cargar = false;

		break;
	case 4:
		
		status = 1;
		start = false;
		out = true;

		break;

	default:
		break;
	}

}

void zones() {  //Cofres: Un solo cofre que se muestra o no dependiendo de si ha sido recogido o no. Este se mueve o se muestra a la posición adecuada dependiendo de la zona.

	switch (zona)
	{
	case 0: //Zona inicial
		NF_CreateTiledBg(0, 0, "zonaInicial"); // Imagen en la pantalla superior
		NF_ShowSprite(0, 4, false);
		if(sariel.x>100 && sariel.x<125 && sariel.y<15 && key) {
			zona = 4;
			sariel.x = 120;
			sariel.y = 180;
		}
		NF_ShowSprite(0, 5, false);
		NF_ShowSprite(0, 6, false);
		NF_ShowSprite(0, 7, false);

		screen_bottom = 145;
		break;
	case 1: //Zona derecha
		NF_CreateTiledBg(0, 0, "zonaDerecha"); // Imagen en la pantalla superior
		if (armor == false) {
			NF_MoveSprite(0, 4, 40, 50);
			NF_ShowSprite(0, 4, true);
		}
		else {
			NF_ShowSprite(0, 4, false);
		}
		NF_ShowSprite(0, 5, false);
		NF_ShowSprite(0, 6, false);
		NF_ShowSprite(0, 7, false);

		screen_right = 230;
		screen_bottom = 145;
		break;
	case 2: //Zona izquierda
		NF_CreateTiledBg(0, 0, "zonaIzquierda"); // Imagen en la pantalla superior
		
		if (lostSoul2.alive) {
			NF_ShowSprite(0, 6, true); //LostSoul
		}

		NF_ShowSprite(0, 5, false);
		NF_ShowSprite(0, 7, false);

		screen_bottom = 145;
		break;
	case 3: //Zona subterranea
		NF_CreateTiledBg(0, 0, "zonaSubterranea"); // Imagen en la pantalla superior

		if (sariel.y > 190) {
			zona = 0;
			sariel.x = 180;
			sariel.y = 60;
		}

		if (key==false) {
			NF_MoveSprite(0, 4, 15, 60);
			NF_ShowSprite(0, 4, true);
		}
		else {
			NF_ShowSprite(0, 4, false);
		}

		if (slime.alive) {
			NF_ShowSprite(0, 7, true); //Slime
		}
		

		NF_ShowSprite(0, 5, false);
		NF_ShowSprite(0, 6, false);
		screen_bottom = 196;
		screen_right = 240;
		screen_left = 10;
		break;

	case 4: //Zona final
		NF_CreateTiledBg(0, 0, "zonaFinal"); // Imagen en la pantalla superior

		if (sword==false) {
			NF_MoveSprite(0, 4, 40, 90);
			NF_ShowSprite(0, 4, true);
		}
		else {
			NF_ShowSprite(0, 4, false);
		}
		
		if (sariel.y > 190) {
			zona = 0;
			sariel.x = 120;
			sariel.y = 25;
		}

		if (throne.alive) {
			NF_ShowSprite(0, 5, true); //Angel
		}

		NF_ShowSprite(0, 6, false);
		NF_ShowSprite(0, 7, false);
		screen_bottom = 196;
		break;
	default:
		break;
	}

}

void combatSetUp(Enemigo e) { //Mostramos Sprite del enemigo en la pantalla superior, su salud y su Palabra si la conocemos. Abajo damos las opciones pertinentes y nuestra salud

	e.x = 100;
	e.y = 70;
	
	if (cargar_combate) {

		mmStart(MOD_COMBAT, MM_PLAY_LOOP);
		mmSetModuleVolume(30);

		NF_ShowSprite(0, sariel.id, false);
		NF_ShowSprite(0, 4, false);

		NF_DeleteTiledBg(0, 0);
		NF_DeleteTiledBg(1, 1);
		
		e.x = 100;
		e.y = 70;
		

		switch (e.id)
		{
		case 9:
			NF_LoadSpriteGfx("sprites/slimeSheet", 9, 64, 64);
			NF_LoadSpritePal("sprites/slimeSheet", 9);
			NF_VramSpriteGfx(0, 9, 9, false);	
			NF_VramSpritePal(0, 9, 9);

			NF_CreateSprite(0, e.id, 9, 9, e.x, e.y);
			NF_MoveSprite(0, e.id, e.x, e.y);
			break;
		case 10:
			NF_LoadSpriteGfx("sprites/throneSheet", 10, 64, 64);
			NF_LoadSpritePal("sprites/throneSheet", 10);
			NF_VramSpriteGfx(0, 10, 10, false);	
			NF_VramSpritePal(0, 10, 10);

			NF_CreateSprite(0, e.id, 10, 10, e.x, e.y);
			NF_MoveSprite(0, e.id, e.x, e.y);
			break;
		case 11:
			NF_LoadSpriteGfx("sprites/lostSoul", 11, 64, 64);
			NF_LoadSpritePal("sprites/lostSoul", 11);
			NF_VramSpriteGfx(0, 11, 11, false);	
			NF_VramSpritePal(0, 11, 11);

			NF_CreateSprite(0, e.id, 11, 11, e.x, e.y);
			NF_MoveSprite(0, e.id, e.x, e.y);
			break;
		case 12:
			NF_LoadSpriteGfx("sprites/lostSoulSheet64", 12, 64, 64);
			NF_LoadSpritePal("sprites/lostSoulSheet64", 12);
			NF_VramSpriteGfx(0, 12, 12, false);	
			NF_VramSpritePal(0, 12, 12);

			NF_CreateSprite(0, e.id, 12, 12, e.x, e.y);
			NF_MoveSprite(0, e.id, e.x, e.y);
			break;
		default:
			break;
		}

		//NF_SpriteFrame(0, 2, 0);

		
	}

	cargar_combate = false;


}

void unloadCombat() {
	
	if (descargar) {
		
		NF_DeleteSprite(0, combat_enemy.id);

		use_word = false;
		word = 0;
		show_soul = false;
		word_selector = 0;

		mmStart(MOD_MAIN, MM_PLAY_LOOP);
	}
	descargar = false;

}

void showMessege(int orden) {

	switch (orden)
	{
	case 0: //Llave
		status_anterior = status;
		temporizador = 0;
		status = 4;

		NF_ClearTextLayer16(1, 0);
		NF_CreateTiledBg(1, 1, "menu");
		NF_WriteText16(1, 0, 3, 3, "Has obtenido la Llave.");
		NF_WriteText16(1, 0, 3, 4, "¿Tendra algun uso?");
		NF_UpdateTextLayers();

		break;
	case 1: //Espada obtenida
		status_anterior = status;
		temporizador = 0;
		status = 4;

		NF_ClearTextLayer16(1, 0);
		NF_CreateTiledBg(1, 1, "menu");
		NF_WriteText16(1, 0, 3, 3, "Has obtenido la espada!");
		NF_WriteText16(1, 0, 3, 4, "Nuevo ataque obtenido.");
		NF_UpdateTextLayers();

		break;
	case 2: //Armadura obtenida

		status_anterior = status;
		temporizador = 0;
		status = 4;

		NF_ClearTextLayer16(1, 0);
		NF_CreateTiledBg(1, 1, "menu");
		NF_WriteText16(1, 0, 3, 3, "Has obtenido la armadura!");
		NF_WriteText16(1, 0, 3, 4, "Tu salud aumenta!");
		NF_WriteText16(1, 0, 3, 5, "Puedes protegerte:");
		NF_WriteText16(1, 0, 3, 6, "accion para evitar");
		NF_WriteText16(1, 0, 3, 7, "el siguiente ataque.");
		NF_UpdateTextLayers();

		sariel.max_health+=10;
		sariel.health+=5;

		
		break;
	default:
		break;
	}

}

bool readingMessege() {
	scanKeys();
	int keys = keysDown();
	if (keys&&temporizador>1) {
		if (keys & KEY_A)
		{
			return true;
		}
	}
}

void ExtraControl() {
	if (terminar_combate==false) {
		terminar_combate = true;
	}
}