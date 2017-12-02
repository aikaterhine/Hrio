#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include<allegro5/allegro_primitives.h>
#include <stdlib.h>
#include <math.h>

const float FPS = 100;
const int SCREEN_W = 1000;
const int SCREEN_H = 500;

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_COLOR BKG_COLOR;
ALLEGRO_FONT *size_32;

typedef struct Nave {
	float x;
	float y;
	float raio;
	float dx;
	float dy;
	ALLEGRO_COLOR cor;
} Nave;

int playing=1;
int qntdnaves=0;
struct Nave *navesec = NULL;
int i, j, xini, yini;
volatile int tempo;

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

    size_32 = al_load_font("arial.ttf", 32, 1);

   	/*registra na fila de eventos que eu quero identificar quando a tela foi alterada */
    al_register_event_source(event_queue, al_get_display_event_source(display));
	/*registra na fila de eventos que eu quero identificar quando o tempo alterou de t para t+1*/
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
	/* Registra mouse na fila de eventos:*/
	al_register_event_source(event_queue, al_get_mouse_event_source());

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
	(*b).dx = 0;
	(*b).dy = 0;
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

void jogo(){

    Nave nave;
    criaNave(&nave, 30);

    Nave naveinimiga;
    criaInimigo(&naveinimiga, 30);

    Nave *navesec = NULL;

    int mod1 = rand()%400;
    int mod2 = rand()%400;

    while(playing !=0){
        tempo++;
        srand((unsigned) time(NULL));

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

           naveinimiga.x += naveinimiga.dx;
           naveinimiga.y += naveinimiga.dy;

           desenhaNave(nave);
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
                printf("perdeu");
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

        naveinimiga.dx = (mod1 - naveinimiga.x)/dist(mod1, naveinimiga.x, mod2, naveinimiga.y);
        naveinimiga.dy = (mod2 - naveinimiga.y)/dist(mod1, naveinimiga.x, mod2, naveinimiga.y);

        al_clear_to_color(BKG_COLOR);
    }
}

int main() {

    if(iniciar() != -1){
        jogo();
    }
    else{
        puts("Fim de Jogo");
    }
        return 0;
}
