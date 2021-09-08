/**********************************************************
* Jogo da Vida (Conway's Game of Life)
*  Implementado com Allegro 5
*
* Fernando S. Pacheco
* fspacheco@gmail.com
*
* Implementação didática, sem otimização
* Usa matriz estática
*
* Versão para web (emscripten)
* O que muda? Como emscripten não abre caixa de diálogo,
* o carregamento de arquivos com a tecla L foi modificado
* para ser um conjunto fixo de 10 arquivos, com as 
* teclas de 0 a 9.
*
* Licença: GNU GPLv3
*
* 2013-2021
***********************************************************/

//Standard size: 100, 64. Bigger blocks, good for presentation: 25, 16
#ifndef WIDTH
#define WIDTH 100
#endif

#ifndef HEIGHT
#define HEIGHT 64
#endif

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "cell_files.h"
//#define ALLEGRO_STATICLINK
//GLOBALS==============================

const uint16_t SCREEN_WIDTH=1000;
const uint16_t SCREEN_HEIGHT=640;
const uint16_t STATUS_BAR_HEIGHT=25;
int WIDTH_FACTOR=SCREEN_WIDTH/WIDTH;
int HEIGH_FACTOR=SCREEN_HEIGHT/HEIGHT;
enum KEYS {F1, PLUS, MINUS, C, L, G, SPACE};
enum STATE {INTRO, USER_ENTRY, FILE_CHOOSER, PLAYING};
bool keys[7] = {false, false, false, false, false, false, false};

uint8_t universe[WIDTH][HEIGHT];
uint8_t underUniverse[WIDTH+2][HEIGHT+2]; //for neighbourhood checking

//prototypes
void InitUniverse();
void drawUniverse(bool grid);
void updateUniverse(void);
void LoadFile(const char* filename);
void fileSelect(ALLEGRO_DISPLAY *disp, ALLEGRO_TIMER *tim);
void showHelpMessageOnScreen(ALLEGRO_FONT *fTitle, ALLEGRO_FONT *fText);
void drawStatusBar(int st, ALLEGRO_FONT *fText, int uppulse, int fps);
void showIntro(ALLEGRO_FONT *fTitle, ALLEGRO_FONT *fSubt, ALLEGRO_FONT *fText, int *drtimer, int uppulse);

int main(void)
{
     //primitive variable
     bool done = false;
     bool grid = false;
     int state=-1;
     bool redraw = true;
     const int FPS = 60;
     bool isGameOver = false;
     int updatepulse=20; // relative to FPS (1 equals FPS; 20 equals 20 times slower than FPS)
     int drawingtimer;

     //object variables

     //Allegro variables
     ALLEGRO_DISPLAY *display = NULL;
     ALLEGRO_EVENT_QUEUE *event_queue = NULL;
     ALLEGRO_TIMER *timer = NULL;
     ALLEGRO_FONT *fontText = NULL;
     ALLEGRO_FONT *fontTitle = NULL;
     ALLEGRO_FONT *fontSubtitle = NULL;
     ALLEGRO_FONT *fontHelp = NULL;
     ALLEGRO_SAMPLE *theme = NULL;
     ALLEGRO_SAMPLE_INSTANCE *themeInstance = NULL;

     //Initialization Functions
     if(!al_init())										//initialize Allegro
          return -1;

     display = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT+STATUS_BAR_HEIGHT);			//create our display object

     if(!display)										//test display object
          return -1;

     al_init_primitives_addon();
     al_install_mouse();
     al_install_keyboard();
     al_init_font_addon();
     al_init_ttf_addon();
     al_init_image_addon();
     al_install_audio();
     al_init_acodec_addon();
     //al_init_native_dialog_addon(); //a partir da versao 5.0.9

     event_queue = al_create_event_queue();
     timer = al_create_timer(1.0 / FPS);

     //introImage  = al_load_bitmap("capa-jogo-vida.png");
     //	al_convert_mask_to_alpha(shipImage, al_map_rgb(255, 0, 255));

     srand(time(NULL));
     InitUniverse();

     fontText = al_load_font("assets/VideoPhreak.ttf", 18, 0);
     if (!fontText) exit(-1);
     fontTitle = al_load_font("assets/Arcade.ttf", 65, 0);
     if (!fontTitle) exit(-1);
     fontSubtitle = al_load_font("assets/Arcade.ttf", 48, 0);
     if (!fontSubtitle) exit(-1);
     fontHelp = al_load_font("assets/Rubik-Regular.ttf", 18, 0);
     if (!fontHelp) exit(-1);

    al_reserve_samples(1);
    theme = al_load_sample("assets/61703__djgriffin__0004-1-audio.wav");
    if (!theme) exit(-1);
    themeInstance = al_create_sample_instance(theme);
    al_attach_sample_instance_to_mixer(themeInstance, al_get_default_mixer());

     al_register_event_source(event_queue, al_get_keyboard_event_source());
     al_register_event_source(event_queue, al_get_mouse_event_source());
     al_register_event_source(event_queue, al_get_timer_event_source(timer));
     al_register_event_source(event_queue, al_get_display_event_source(display));

     al_start_timer(timer);
     drawingtimer=0;
     state=INTRO;
     while(!done) {
          ALLEGRO_EVENT ev;
          al_wait_for_event(event_queue, &ev);

          if(ev.type == ALLEGRO_EVENT_TIMER) {
               redraw = true;
               drawingtimer++;
          } else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
               done = true;
          } else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
               if (ev.mouse.button & 1) { //left button
                    //passar coordenada do mouse para posicao na matriz
                    if (state==USER_ENTRY) {
                         universe[ev.mouse.x/WIDTH_FACTOR][ev.mouse.y/HEIGH_FACTOR] =
                              !universe[ev.mouse.x/WIDTH_FACTOR][ev.mouse.y/HEIGH_FACTOR];
                         //                 printf("passei para universe!!!\n");
                    }
                    //al_draw_textf(font18, al_map_rgb(255, 0, 255), 5, 5, 0, "X: %d, Y: %d", ev.mouse.x, ev.mouse.y);
               }

          } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
               switch(ev.keyboard.keycode) {
               case ALLEGRO_KEY_ESCAPE:
                    done = true;
                    break;
               case ALLEGRO_KEY_C:
                    keys[C] = true;
                    InitUniverse();
                    state=USER_ENTRY;
                    break;
               case ALLEGRO_KEY_L:
                    keys[L] = true;
                    state=FILE_CHOOSER;
                    break;
               case ALLEGRO_KEY_G:
                    keys[G] = true;
                    grid=!grid;
                    break;
               case ALLEGRO_KEY_EQUALS:
               case ALLEGRO_KEY_PAD_PLUS:
                   // keys[PLUS] = true;
                    updatepulse--; //decreasing updatepulse, it will speed up the update
                    if (updatepulse<1) {
                         updatepulse=1;
                    }
                    break;
               case ALLEGRO_KEY_MINUS:
               case ALLEGRO_KEY_PAD_MINUS:
                   // keys[MINUS] = true;
                    updatepulse++;
                    break;
               case ALLEGRO_KEY_F1:
                    keys[F1] = true;
                    showHelpMessageOnScreen(fontTitle, fontHelp);
                    break;
               case ALLEGRO_KEY_SPACE:
                    keys[SPACE] = true;
                    //printf("state: %d\n",state);
                    switch (state) {
                    case INTRO:
                         state=USER_ENTRY;
                         al_stop_sample_instance(themeInstance);
                         break;
                    case USER_ENTRY:
                         state=PLAYING;
                         break;
                    case PLAYING:
                         state=USER_ENTRY;
                         break;
                    }
                    break;
               case ALLEGRO_KEY_0:
                    LoadFile(CELL_FILE[0]);
                    break;
               case ALLEGRO_KEY_1:
                    LoadFile(CELL_FILE[1]);
                    break;
               case ALLEGRO_KEY_2:
                    LoadFile(CELL_FILE[2]);
                    break;
               case ALLEGRO_KEY_3:
                    LoadFile(CELL_FILE[3]);
                    break;
               case ALLEGRO_KEY_4:
                    LoadFile(CELL_FILE[4]);
                    break;
               case ALLEGRO_KEY_5:
                    LoadFile(CELL_FILE[5]);
                    break;                    
               case ALLEGRO_KEY_6:
                    LoadFile(CELL_FILE[6]);
                    break;
               case ALLEGRO_KEY_7:
                    LoadFile(CELL_FILE[7]);
                    break;
               case ALLEGRO_KEY_8:
                    LoadFile(CELL_FILE[8]);
                    break;
               case ALLEGRO_KEY_9:
                    LoadFile(CELL_FILE[9]);
                    break;                    
               }
          } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
               switch(ev.keyboard.keycode) {
               case ALLEGRO_KEY_ESCAPE:
                    done = true;
                    break;
               case ALLEGRO_KEY_SPACE:
                    keys[SPACE] = false;
                    break;
               case ALLEGRO_KEY_C:
                    keys[C] = false;
                    break;
               case ALLEGRO_KEY_L:
                    keys[L] = false;
                    break;
               case ALLEGRO_KEY_G:
                    keys[G] = false;
                    break;
               case ALLEGRO_KEY_EQUALS:
               case ALLEGRO_KEY_PAD_PLUS:
                  //  keys[PLUS] = false;
                    break;
               case ALLEGRO_KEY_MINUS:
               case ALLEGRO_KEY_PAD_MINUS:
                 //   keys[MINUS] = false;
                    break;
               case ALLEGRO_KEY_F1:
                    keys[F1] = false;
                    break;
               }
          }

          if(redraw && al_is_event_queue_empty(event_queue)) {
               redraw = false;
               al_clear_to_color(al_map_rgb(0,0,0));

               if(!isGameOver) {
                    switch (state) {
                    case INTRO:
                         showIntro(fontTitle, fontSubtitle, fontText, &drawingtimer, updatepulse);
                         al_play_sample_instance(themeInstance);
                         break;
                    case PLAYING:
                         if (drawingtimer>=updatepulse) {
                              updateUniverse();
                              drawingtimer=0;
                         }
                         //fall trough to drawUniverse
                    case USER_ENTRY:
                         drawUniverse(grid);//vale para user_entry e playing
                         drawStatusBar(state, fontText, updatepulse, FPS);
                         break;
                    case FILE_CHOOSER:
                         fileSelect(display, timer);
                         state=USER_ENTRY;
                         break;
                    }

               }
               al_flip_display();
          }
     }


     al_destroy_event_queue(event_queue);
     al_destroy_timer(timer);
//    al_shutdown_native_dialog_addon(); // a partir da versao 5.0.9
     al_destroy_font(fontText);
     al_destroy_font(fontTitle);
     al_destroy_font(fontSubtitle);
     al_destroy_sample(theme);
     al_destroy_sample_instance(themeInstance);
     al_destroy_display(display);

     return 0;
}

void InitUniverse(void)
{
     for (int j=0; j<WIDTH; j++) {
          for (int k=0; k<HEIGHT; k++) {
               universe[j][k]=0;
          }
     }
 }

void drawUniverse(bool grid)
{
     if (grid) {
          al_draw_filled_rectangle(0,0, SCREEN_WIDTH, SCREEN_HEIGHT, al_map_rgb(85,115,130));
     }
     for (int j=0; j<WIDTH; j++) {
          for (int k=0; k<HEIGHT; k++) {
               if (universe[j][k]==0) {
                    al_draw_filled_rectangle(j*WIDTH_FACTOR, k*HEIGH_FACTOR,
                                             j*WIDTH_FACTOR+WIDTH_FACTOR-1, k*HEIGH_FACTOR+HEIGH_FACTOR-1,
                                             al_map_rgb(0, 0, 0));
               } else if (universe[j][k]==1) {
                    al_draw_filled_rectangle(j*WIDTH_FACTOR, k*HEIGH_FACTOR,
                                             j*WIDTH_FACTOR+WIDTH_FACTOR-1, k*HEIGH_FACTOR+HEIGH_FACTOR-1,
                                             al_map_rgb(0, 255, 40));
               }
          }
     }
}

void drawStatusBar(int st,ALLEGRO_FONT *fText, int uppulse, int fps)
{
     char text[20];
     switch (st) {
     case USER_ENTRY:
     case FILE_CHOOSER:
          strcpy(text, "User Entry");
          break;
     case PLAYING:
          strcpy(text, "Running");
          break;
     }
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 30, SCREEN_HEIGHT+2, 0,
                   "Status: %s", text);

     al_draw_textf(fText, al_map_rgb(255, 255, 255), SCREEN_WIDTH-130, SCREEN_HEIGHT+2, ALLEGRO_ALIGN_LEFT,
                   "Speed: %d", 101-uppulse);//não é exatamente a fórmula correta, mas dá um sentimento

}


/* celulas redondas
void drawUniverse()
{
    for (int j=0; j<WIDTH; j++)
    {
        for (int k=0; k<HEIGHT; k++)
        {
            if (universe[j][k]==0)
            {
                al_draw_filled_rectangle(j*WIDTH_FACTOR, k*HEIGH_FACTOR,
                  j*WIDTH_FACTOR+WIDTH_FACTOR, k*HEIGH_FACTOR+HEIGH_FACTOR,
                  al_map_rgb(255, 255, 0));
            }
            else if (universe[j][k]==1)
            {
                al_draw_filled_rectangle(j*WIDTH_FACTOR, k*HEIGH_FACTOR,
                    j*WIDTH_FACTOR+WIDTH_FACTOR, k*HEIGH_FACTOR+HEIGH_FACTOR,
                    al_map_rgb(255, 255, 0));
                al_draw_filled_circle(j*WIDTH_FACTOR+WIDTH_FACTOR/2, k*HEIGH_FACTOR+HEIGH_FACTOR/2, WIDTH_FACTOR/2, al_map_rgb(255, 0, 0));
            }
        }
    }
}
*/


void updateUniverse()
{
     int neighbour;
     int j;
     int k;
     //copyUniverse(universe, newUniverse);
     j=0;
     for (k=0; k<HEIGHT+2; k++) {
          underUniverse[j][k]=0;
     }
     j=WIDTH+1;
     for (k=0; k<HEIGHT+2; k++) {
          underUniverse[j][k]=0;
     }
     k=0;
     for (j=0; j<WIDTH+2; j++) {
          underUniverse[j][k]=0;
     }
     k=HEIGHT+1;
     for (j=0; j<WIDTH+2; j++) {
          underUniverse[j][k]=0;
     }

     for (int j=0; j<WIDTH; j++) {
          for (int k=0; k<HEIGHT; k++) {
               underUniverse[j+1][k+1]=universe[j][k];
          }
     }
     //até aqui, era só para preparar a matriz que vai ser testada (underUniverse)

     for (int j=1; j<WIDTH+1; j++) {
          for (int k=1; k<HEIGHT+1; k++) {
               neighbour=underUniverse[j-1][k-1] + underUniverse[j][k-1] + underUniverse[j+1][k-1] +
                         underUniverse[j-1][k] +                           underUniverse[j+1][k] +
                         underUniverse[j-1][k+1] + underUniverse[j][k+1] + underUniverse[j+1][k+1]; //neighbours of (j,k)
               //printf("(%d,%d): %d\n", j,k,neighbour);
               if (neighbour==3 || (underUniverse[j][k]==1 && neighbour==2)) {
                    universe[j-1][k-1]=1; //it will born
               } else {
                    universe[j-1][k-1]=0; //it will die
               }
          }
     }

}

void LoadFile(const char *filename)
{
     FILE *fp;
     char line[WIDTH];
     char name[30];
     fp=fopen(filename, "r");
     if (!fp) exit(-1);
     fscanf(fp, "%s", line);
     fscanf(fp, "%s", name);
     if (strcmp(line, "!Name:")!=0) exit(-1);
     fscanf(fp, "%s", line); //second line
     if (strcmp(line, "!")!=0) exit(-1);
     int nlin=0;
     int ncol;
     while(fscanf(fp, "%s", line)!=EOF) {
          ncol=strlen(line);
          if (nlin>HEIGHT) exit(-1);
          if (ncol>WIDTH) exit(-1);
          for (int j=0; j<ncol; j++) {
               universe[j][nlin]= (line[j]=='O'?1:0);
          }
          //printf("Linha: %s\n", line);
          nlin++;
     }
     fclose(fp);
}

void fileSelect(ALLEGRO_DISPLAY *disp, ALLEGRO_TIMER *tim)
{
/*     ALLEGRO_FILECHOOSER *file_load_dialog = NULL;
     const char *rp = NULL;

     file_load_dialog = al_create_native_file_dialog("cells/",
                        "Load game of life file", "*.cells;*.*",
                        ALLEGRO_FILECHOOSER_FILE_MUST_EXIST);
     if(file_load_dialog) {
          al_stop_timer(tim);
          al_show_native_file_dialog(disp, file_load_dialog);
          rp = al_get_native_file_dialog_path(file_load_dialog, 0);
          if(rp) {
               printf("File: %s\n", rp);
               LoadFile(rp);

          }
          al_destroy_native_file_dialog(file_load_dialog);
          al_start_timer(tim);
     }
     */
}

void showHelpMessageOnScreen(ALLEGRO_FONT *fTitle, ALLEGRO_FONT *fText)
{
     al_draw_textf(fTitle, al_map_rgb(255, 255, 255), SCREEN_WIDTH/2-80, 20, 0, "Help");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 70, 0, 
      "Jogo da Vida (Conway's Game of Life) implementa um autômato celular.");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 95, 0,
      "Esta versão foi desenvolvida em C, usando a biblioteca Allegro 5.");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 120, 0,
      "F1: este Help");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 145, 0,
      "Espaço: alterna entre modos de edição e evolução");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 170, 0,
      "Mouse: cria ou apaga células no modo de edição");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 195, 0,
      "G: alterna apresentação da grade (grid)");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 220, 0,
      "C: extermina todas as células");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 245, 0,
      "0 a 9: carrega cells de http://www.bitstorm.org/gameoflife/lexicon/");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 280, 0,
      "Desenvolvido por Fernando S. Pacheco (fspacheco@gmail.com)");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 305, 0,
      "Licença: GNU GPLv3");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 330, 0,
      "Música de abertura:");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 355, 0,
      "http://www.freesound.org/people/djgriffin/sounds/61703/");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 380, 0,
      "CC BY-NC 3.0");
      al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, 420, 0,
      "Versão Web compilada com Emscripten");
  
     al_flip_display();
     al_rest(5.0);
}


void showIntro(ALLEGRO_FONT *fTitle, ALLEGRO_FONT *fSubt, ALLEGRO_FONT *fText, int *drtimer, int uppulse)
{
     al_draw_textf(fTitle, al_map_rgb(255, 255, 255), SCREEN_WIDTH/2-200, SCREEN_HEIGHT/2-80, 0, "Jogo da Vida");
     al_draw_textf(fSubt, al_map_rgb(255, 255, 255), SCREEN_WIDTH/2-150, SCREEN_HEIGHT/2-30, 0, "Conway's Game of Life");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, SCREEN_HEIGHT/2+160, 0,
                   "Este é um jogo sem jogador (0-player game),");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, SCREEN_HEIGHT/2+180, 0,
                   "em que um autômato celular segue uma regra bastante simples:");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, SCREEN_HEIGHT/2+200, 0,
                   "- uma nova célula é criada se tiver três vizinhas vivas e");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, SCREEN_HEIGHT/2+220, 0,
                   "- uma célula sobrevive se tiver duas ou três vizinhas.");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, SCREEN_HEIGHT/2+250, 0,
                   "Aperte [espaco] e vá para o Universo...");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 146, SCREEN_HEIGHT/2+252, 0,
                   ","); //cedilha de espaço
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 20, SCREEN_HEIGHT/2+270, 0,
                   "Crie uma colônia e veja sua evolucão!");
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 351, SCREEN_HEIGHT/2+272, 0,
                   ","); //cedilha de evolução
     al_draw_textf(fText, al_map_rgb(255, 255, 255), 550, SCREEN_HEIGHT/2+270, 0,
                   "Desenvolvido por Fernando S. Pacheco");
     static uint8_t intro_color_r=0;
     static int8_t fator=10;
     if (*drtimer>=uppulse) {
          intro_color_r+=fator;
          if (intro_color_r<=10) {
               fator=+10;
          }
          if (intro_color_r>=240) {
               fator=-10;
          }
          *drtimer=0;
     }
     al_draw_filled_rectangle(20, 20, 40, 40, al_map_rgb(intro_color_r, 0, 0));
     al_draw_filled_rectangle(60, 60, 80, 80, al_map_rgb(intro_color_r, 255-intro_color_r, 0));
     al_draw_filled_rectangle(20, 100, 40, 120, al_map_rgb(intro_color_r, 0, 0));
     al_draw_filled_rectangle(SCREEN_WIDTH-20, SCREEN_HEIGHT-20, SCREEN_WIDTH-40, SCREEN_HEIGHT-40, al_map_rgb(255-intro_color_r, 0, 0));
     al_draw_filled_rectangle(SCREEN_WIDTH-60, SCREEN_HEIGHT-60, SCREEN_WIDTH-80, SCREEN_HEIGHT-80, al_map_rgb(intro_color_r, intro_color_r, 0));
     al_draw_filled_rectangle(SCREEN_WIDTH-20, SCREEN_HEIGHT-100, SCREEN_WIDTH-40, SCREEN_HEIGHT-120, al_map_rgb(255-intro_color_r, 0, 0));
}
