#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define MAX_TAM 50
#define MAX_PESSOAS 6

typedef struct Nave {
	float x;
	float y;
	float raio;
	float dx;
	float dy;
	ALLEGRO_COLOR cor;
} Nave;

const float FPS = 100;
const int SCREEN_W = 100;
const int SCREEN_H = 200;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *fonte = NULL;
ALLEGRO_COLOR BKG_COLOR;

struct Nave *navesec = NULL;
int i, j, xini, yini, x, y, temp,qntdnaves=0, playing=1,Rpont[MAX_PESSOAS];
volatile long long int tempo =0;
bool concluido = false, sair = false;
char nome[MAX_PESSOAS][17], *nomeaux, buf[MAX_TAM], str[17];

float calculaRaio(float area1, float area2){
    float areat = area1+area2;
    float raio = sqrt((areat/3.1415));
    return raio;
}

void dispararAleatorio(Nave **navesec){
    Nave *redim_naves;
    int rand1, rand2;

     redim_naves = (Nave *)realloc(*navesec, ((qntdnaves + 1) * sizeof(Nave)));
     *navesec = redim_naves;

     if(*navesec == NULL){
        printf("\nERRO AO ALOCAR MEMORIA.");
    }
	else{
        qntdnaves++;

		rand1 = rand()%100;
		rand2 = rand()%100;


        (*navesec)[qntdnaves].raio = 4;
        (*navesec)[qntdnaves].dx = 2;
        (*navesec)[qntdnaves].dy = 2;
        (*navesec)[qntdnaves].cor = al_map_rgb(140,0,26);

        (*navesec)[qntdnaves].x = rand1;
		(*navesec)[qntdnaves].y = rand2;
    }
}

void detectarColisaoPrincipal(Nave *nave, Nave **navesec){
    float areaP, areaS;
    areaP = pow((*nave).raio, 2) * 3.1415;
    for(i=0; i<qntdnaves; i++){
        if( ((*nave).x + (*nave).raio > (*navesec)[i].x && (*nave).x < (*navesec)[i].x + (*navesec)[i].raio)
           && ((*nave).y < (*navesec)[i].y + (*navesec)[i].raio && (*nave).y > (*navesec)[i].y)) {
                   areaS = pow((*navesec)[i].raio, 2) * 3.1415;
                   if(areaP>areaS){
                        (*nave).raio = calculaRaio(areaS, areaP);
                        for(j=i; j<qntdnaves; j++){
                            (*navesec)[j] = (*navesec)[j+1];
                        }
                        qntdnaves--;
                    }
        }
    }
}

void detectarColisaoSecundaria(Nave **navesec){
    float areaP, areaS;
    int aux;
    for(i=0; i<qntdnaves; i++){
        for(j=0; j< qntdnaves; j++){
            if(((*navesec)[i].x + (*navesec)[i].raio > (*navesec)[j].x && (*navesec)[i].x < (*navesec)[j].x + (*navesec)[j].raio)
               && ((*navesec)[i].y < (*navesec)[j].y + (*navesec)[j].raio && (*navesec)[i].y > (*navesec)[j].y)){

                    areaP = pow((*navesec)[i].raio, 2) * 3.1415;
                    areaS = pow((*navesec)[j].raio, 2) * 3.1415;

                    if(areaP>areaS){
                            (*navesec)[i].raio = calculaRaio(areaP, areaS);
                            for(aux=j ; aux<qntdnaves; aux++){
                                (*navesec)[aux] = (*navesec)[aux+1];
                            }
                            qntdnaves--;
                        }
               }
        }
    }
}

int detectarColisaoInimigo(Nave *nave, Nave *inim){
float areaP, areaS;
    areaP = pow((*nave).raio, 2) * 3.1415;
        if( ((*nave).x + (*nave).raio > (*inim).x && (*nave).x < (*inim).x + (*inim).raio)
           && ((*nave).y < (*inim).y + (*inim).raio && (*nave).y > (*inim).y)) {
                   areaS = pow((*inim).raio, 2) * 3.1415;
                   if(areaS>areaP){
                        (*inim).raio = calculaRaio(areaP, areaS);
                        return 1;
                    }
                    else{
                        return 0;
                    }
        }
}

int iniciar(){
	if(!al_init()) {
      fprintf(stderr, "Falha inicializacao do Allegro!\n");
      return -1;
   }

    al_init_font_addon();
    al_init_ttf_addon();

    if(!al_init_primitives_addon()){
		fprintf(stderr, "Falha inicializacao de primitivos!\n");
        return -1;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);

   if(!display) {
      fprintf(stderr, "Falha criacao do display!\n");
      return -1;
   }

   timer = al_create_timer(1.0 / FPS);

   if(!timer) {
      fprintf(stderr, "Falha criacao do time!\n");
      return -1;
   }

   event_queue = al_create_event_queue();

    if(!event_queue) {
      fprintf(stderr, "Falha criacao fila de eventos!\n");
      return -1;
   }

	if(!al_install_mouse()){
		fprintf(stderr, "Falha inicializacao do mouse!\n");
		return -1;
	}

    if(!al_install_keyboard()){
		fprintf(stderr, "Falha inicializacao do teclado!\n");
		return -1;
	}

    fonte = al_load_ttf_font("comic.ttf", 42, 0);
    if (!fonte)
    {
        fprintf(stderr, "Falha ao carregar \"comic.ttf\".\n");
        return false;
    }

   	/*registra na fila de eventos que eu quero identificar quando a tela foi alterada */
    al_register_event_source(event_queue, al_get_display_event_source(display));
	/*registra na fila de eventos que eu quero identificar quando o tempo alterou de t para t+1*/
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
	/* Registra mouse na fila de eventos:*/
	al_register_event_source(event_queue, al_get_mouse_event_source());
	/* Registra teclado na fila de eventos:*/
	al_register_event_source(event_queue, al_get_keyboard_event_source());

	BKG_COLOR = al_map_rgb(255,255,255);
    /*avisa o allegro que agora eu quero modificar as propriedades da tela*/
    al_set_target_bitmap(al_get_backbuffer(display));
	/*colore a tela de preto (rgb(0,0,0))*/
    al_clear_to_color(BKG_COLOR);
}

void criaNave(Nave *b, int r) {
	(*b).y = SCREEN_H/2;
	(*b).x = SCREEN_W/2;
	(*b).cor = al_map_rgb(140,0,26);
	(*b).raio = r;
	(*b).dx = 0;
	(*b).dy = 0;
}

void desenhaNave(Nave n) {
	al_draw_filled_circle(n.x, n.y, n.raio, n.cor);
}

void criaInimigo(Nave *b, int r){
	(*b).y = rand()%100;
	(*b).x = rand()%100;
	(*b).cor = al_map_rgb(218,112,214);
	(*b).raio = r;
	(*b).dx = 1;
	(*b).dy = -1;
}

float dist(float x1, float x2, float y1, float y2) {
	return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}

int corAleatoria(){
    return rand()%255;
}

void disparar(Nave *nave, Nave **navesec, int evx, int evy){
    int areat, raio;
    Nave *redim_naves;

	if(*navesec == NULL){
        *navesec = (Nave *)malloc(sizeof(Nave));
	}
	else{
        redim_naves = (Nave *)realloc(*navesec, ((qntdnaves + 1) * sizeof(Nave)));
        *navesec = redim_naves;
	}

    if(*navesec == NULL){
        printf("\nERRO AO ALOCAR MEMORIA.");
    }
	else{
        areat = (pow((*nave).raio, 2) * 3.1415) - (pow((*navesec)[qntdnaves].raio, 2) * 3.1415);
        raio = sqrt((areat/3.1415));
        (*navesec)[qntdnaves].raio = (((*nave).raio)/4);
        (*nave).dx = -((*nave).dx + 0.5);
        (*nave).dy = -((*nave).dy + 0.5);
        (*navesec)[qntdnaves].dx = (evx - (*nave).x)/dist(evx, (*nave).x, evy, (*nave).y);
        (*navesec)[qntdnaves].dy = (evy - (*nave).y)/dist(evx, (*nave).x, evy, (*nave).y);
        (*navesec)[qntdnaves].cor = al_map_rgb(corAleatoria(), corAleatoria(), corAleatoria());
        (*navesec)[qntdnaves].x = (*nave).x + (*nave).raio;
		(*navesec)[qntdnaves].y = (*nave).y;
		(*nave).raio = raio;
        qntdnaves++;
    }
}

void desenhaDisparo(Nave navesec){
    al_draw_filled_circle(navesec.x, navesec.y, navesec.raio, navesec.cor);
}

void finalizar(){
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
}

void manipular_entrada(ALLEGRO_EVENT evento){
    if (evento.type == ALLEGRO_EVENT_KEY_CHAR)
    {
        if (strlen(str) <= 16)
        {
            char temp[] = {evento.keyboard.unichar, '\0'};
            if (evento.keyboard.unichar == ' ')
            {
                strcat(str, temp);
            }
            else if (evento.keyboard.unichar >= '0' &&
                     evento.keyboard.unichar <= '9')
            {
                strcat(str, temp);
            }
            else if (evento.keyboard.unichar >= 'A' &&
                     evento.keyboard.unichar <= 'Z')
            {
                strcat(str, temp);
            }
            else if (evento.keyboard.unichar >= 'a' &&
                     evento.keyboard.unichar <= 'z')
            {
                strcat(str, temp);
            }
        }

        if (evento.keyboard.keycode == ALLEGRO_KEY_BACKSPACE && strlen(str) != 0)
        {
            str[strlen(str) - 1] = '\0';
        }
    }
}

void exibir_texto_centralizado(){
    if (strlen(str) > 0)
    {
        al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2,
                     (SCREEN_H - al_get_font_ascent(fonte)) / 2,
                     ALLEGRO_ALIGN_CENTRE, str);
    }
}

void jogo(){

    Nave nave;
    criaNave(&nave, 30);

    Nave naveinimiga;
    criaInimigo(&naveinimiga, 30);

    Nave *navesec = NULL;

    int pont;

    strcpy(str, "");

    FILE *arq;
    arq = fopen("ranking.txt", "r");

    if(arq!=NULL){
        while(feof(arq)==NULL){
            fgets(buf, MAX_TAM, arq);
            nomeaux = strtok(buf, "|");
            strcpy(nome[x],nomeaux);
            Rpont[x] = atoi(strtok(NULL, "|"));
            x++;
        }

        for(i=0; i<MAX_PESSOAS; i++){
            for(j=0; j<MAX_PESSOAS; j++){
                if(Rpont[i]>Rpont[j]){
                    temp = Rpont[i];
                    Rpont[i] = Rpont[j];
                    Rpont[j] = temp;
                    strcpy(nomeaux, nome[i]);
                    strcpy(nome[i], nome[j]);
                    strcpy(nome[j], nomeaux);
                }
            }
        }
    }
    else{
        printf("Arquivo inexistente.");
    }

    fclose(arq);

    arq = fopen("ranking.txt", "w");
    if(arq != NULL){
        for(x=0; x<5; x++){
            fprintf(arq, "%s|%d\n", nome[x], Rpont[x]);
        }
    }
    else{
        printf("Arquivo inexistente.");
    }

    for(x=0; x<5; x++){
        printf("%s %d\n", nome[x], Rpont[x]);
    }

    srand((unsigned) time(NULL));

    while(playing !=0){
        tempo++;

        ALLEGRO_EVENT ev;
        /*inicia o temporizador*/
        al_start_timer(timer);

        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            playing = 0;
        }
        else if(ev.type == ALLEGRO_EVENT_TIMER) {

          if(nave.x > SCREEN_W || nave.x < 0 || naveinimiga.x > SCREEN_W || naveinimiga.x < 0){
              nave.dx = -nave.dx;
              naveinimiga.dx = -naveinimiga.dx;
          }
          if(nave.y > SCREEN_H || nave.y < 0 || naveinimiga.y > SCREEN_H || naveinimiga.y < 0){
              nave.dy = - nave.dy;
              naveinimiga.dy = -naveinimiga.dy;
          }

           nave.x += nave.dx;
           nave.y += nave.dy;

           desenhaNave(nave);

           naveinimiga.x += naveinimiga.dx;
           naveinimiga.y += naveinimiga.dy;

           desenhaNave(naveinimiga);

        if(navesec!=NULL){
            for(i=0; i<qntdnaves; i++){

                if(navesec[i].x > SCREEN_W || navesec[i].x < 0){
                    navesec[i].dx = -navesec[i].dx;
                }

                if(navesec[i].y > SCREEN_H || navesec[i].y < 0){
                    navesec[i].dy = -navesec[i].dy;
                }

                navesec[i].x += navesec[i].dx;
                navesec[i].y += navesec[i].dy;
                desenhaDisparo(navesec[i]);
            }

            if((tempo/1000)%30==0){
                //dispararAleatorio(&navesec);
                for(i=0; i<qntdnaves; i++){
                    desenhaDisparo(navesec[i]);
                }
            }

            if(detectarColisaoInimigo(&nave, &naveinimiga)==1){
                playing = 0;
            }
            else{
                detectarColisaoPrincipal(&nave, &navesec);
                detectarColisaoPrincipal(&naveinimiga, &navesec);
                detectarColisaoSecundaria(&navesec);
            }
        }
        al_flip_display();
    }

        else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            nave.dx = (ev.mouse.x - nave.x)/dist(ev.mouse.x, nave.x, ev.mouse.y, nave.y);
            nave.dy = (ev.mouse.y - nave.y)/dist(ev.mouse.x, nave.x, ev.mouse.y, nave.y);

            if((nave.raio>8)){
                disparar(&nave, &navesec, ev.mouse.x, ev.mouse.y);
                for(i=0; i<qntdnaves; i++){
                    desenhaDisparo(navesec[i]);
                }
            }
        }
        al_clear_to_color(BKG_COLOR);
    }

    al_clear_to_color(al_map_rgb(0,0,0));
    tempo = al_get_timer_count(timer)/FPS;

    for(x=0, y=0; x<MAX_PESSOAS; x++){
        if(Rpont[x]>tempo){
            y++;
        }
    }

    printf("%d", y);

    if(y<=5){
        while (!sair){
            while (!al_is_event_queue_empty(event_queue))
            {
                ALLEGRO_EVENT evento;
                al_wait_for_event(event_queue, &evento);

                if (!concluido)
                {
                    manipular_entrada(evento);

                    if (evento.type == ALLEGRO_EVENT_KEY_DOWN && evento.keyboard.keycode == ALLEGRO_KEY_ENTER)
                    {
                        concluido = true;
                    }
                }

                if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                {
                    sair = true;
                }
            }

            al_draw_bitmap(display, 0, 0, 0);

            if (!concluido)
            {
                al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2,
                            (SCREEN_H / 2 - al_get_font_ascent(fonte)) / 2,
                            ALLEGRO_ALIGN_CENTRE, "Melhor Pontuação! Nome:");
            }
            else
            {
                al_draw_text(fonte, al_map_rgb(255, 255, 255), SCREEN_W / 2,
                            (SCREEN_H / 2 - al_get_font_ascent(fonte)) / 2,
                            ALLEGRO_ALIGN_CENTRE, "1º Lugar");
            }

            exibir_texto_centralizado();

            al_flip_display();
        }
    }
    finalizar();
}

int main() {

    if(iniciar() != -1){
        jogo();
    }
    else{
        puts("Erro de Inicializacao.");
    }
        return 0;
}
