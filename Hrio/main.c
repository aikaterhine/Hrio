#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#define MAX_PESSOAS 6
#define MAX_TAM 50

typedef struct Nave {
	float x;
	float y;
	float raio;
	float dx;
	float dy;
	ALLEGRO_COLOR cor;
} Nave;

const int SCREEN_W = 800;
const int SCREEN_H = 500;
const float FPS = 100;

ALLEGRO_COLOR BKG_COLOR, BKG_COLOR_MSSG;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *fonte = NULL;

int i, j, xini, yini, x, y, temp,qntdnaves=0, playing=1,Rpont[MAX_PESSOAS], qntdjogadores=0, menorTempo=0;
char nome[MAX_PESSOAS][17], *nomeaux, buf[MAX_TAM], str[17];
bool concluido = false, sair = false;
struct Nave *navesec = NULL;
volatile long long int tempo=0;
float evx, evy, dx=0.0, dy=0.0;

int corAleatoria(){
    return rand()%255;
}

float calculaRaio(float area1, float area2){
    float areat = area1+area2;
    float raio = sqrt((areat/3.1415));
    return raio;
}

void dispararAleatorio(Nave **navesec){
    int areat, raio;
    Nave *redim_naves;

	if(*navesec == NULL){
        *navesec = (Nave *)malloc(sizeof(Nave));
	}
	else{
        redim_naves = (Nave *)realloc(*navesec, ((qntdnaves + 1) * sizeof(Nave)));
        *navesec = redim_naves;
        qntdnaves++;
	}
    if(*navesec == NULL){
        printf("\nERRO AO ALOCAR MEMORIA2.");
    }
	else{
        (*navesec)[qntdnaves].raio = 4;
        (*navesec)[qntdnaves].dx = 2;
        (*navesec)[qntdnaves].dy = 2;
        (*navesec)[qntdnaves].cor = al_map_rgb(corAleatoria(), corAleatoria(), corAleatoria());
        (*navesec)[qntdnaves].x = 10;
		(*navesec)[qntdnaves].y = 10;
    }
}

int detectarColisaoPrincipal(Nave *nave, Nave **navesec){
    float areaP, areaS, dist, sRaio;
    areaP = pow((*nave).raio, 2) * 3.1415;
    for(i=0; i<qntdnaves; i++){
        dist = sqrt(pow((*nave).x - (*navesec)[i].x, 2) + pow((*nave).y - (*navesec)[i].y, 2));
        sRaio = (*nave).raio + (*navesec)[i].raio;
        if(dist<sRaio) {
                   areaS = pow((*navesec)[i].raio, 2) * 3.1415;
                   if(areaP>areaS){
                        (*nave).raio = calculaRaio(areaS, areaP);
                        for(j=i; j<qntdnaves; j++){
                            (*navesec)[j] = (*navesec)[j+1];
                        }
                        (*nave).dx-=0.9;
                        (*nave).dy-=0.9;
                        qntdnaves--;
                        return 1;
                    }
                    if(areaS>areaP){
                        return 0;
                    }
        }
    }
}

void detectarColisaoSecundaria(Nave **navesec){
    float areaP, areaS, dist, sRaio;
    int aux;
    for(i=0; i<qntdnaves; i++){
        for(j=0; j< qntdnaves; j++){
            dist = sqrt(pow((*navesec)[i].x - (*navesec)[j].x, 2) + pow((*navesec)[i].y - (*navesec)[j].y, 2));
            sRaio = (*navesec)[j].raio + (*navesec)[i].raio;
                if(dist<sRaio) {

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
float areaP, areaS, dist, sRaio;
    areaP = pow((*nave).raio, 2) * 3.1415;
    dist = sqrt(pow((*nave).x - (*inim).x, 2) + pow((*nave).y - (*inim).y, 2));
    sRaio = (*nave).raio + (*inim).raio;
        if(dist<sRaio) {
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
        return -1;
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
	BKG_COLOR_MSSG = al_map_rgb(10,10,10);
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
	(*b).y = 10 + rand() % (SCREEN_H - 20);
	(*b).x = 10 + rand() % (SCREEN_W - 20);
	(*b).cor = al_map_rgb(218,112,214);
	(*b).raio = r;
	(*b).dx = 1;
	(*b).dy = -1;
}

float dist(float x1, float x2, float y1, float y2) {
	return sqrt(pow(x1-x2,2) + pow(y1-y2,2));
}

void disparar(Nave *nave, Nave **navesec, int evx, int evy, int id){
    float areat, raio;
    Nave *redim_navess;

	if(*navesec == NULL){
        *navesec = (Nave *)malloc(sizeof(Nave));
	}
	else{
        redim_navess = (Nave *)realloc(*navesec, ((qntdnaves + 1) * sizeof(Nave)));
        *navesec = redim_navess;
	}

    if(*navesec == NULL){
        printf("\nERRO AO ALOCAR MEMORIA.");
    }
	else{
        if(id == 0){
            (*navesec)[qntdnaves].raio = (((*nave).raio)/4);
            areat =  (pow((*nave).raio, 2) * 3.1415) - (pow((*navesec)[qntdnaves].raio, 2) * 3.1415);
            raio = sqrt((areat/3.1415));
            (*navesec)[qntdnaves].dx = (evx - (*nave).x)/dist(evx, (*nave).x, evy, (*nave).y);
            (*navesec)[qntdnaves].dy = (evy - (*nave).y)/dist(evx, (*nave).x, evy, (*nave).y);
            (*navesec)[qntdnaves].cor = al_map_rgb(corAleatoria(), corAleatoria(), corAleatoria());
            (*navesec)[qntdnaves].x = (*nave).x + (*nave).raio + 7;
            (*navesec)[qntdnaves].y = (*nave).y + (*nave).raio;
            (*nave).raio = raio;
            qntdnaves++;
        }
        if(id == 1){
            (*navesec)[qntdnaves].raio = 5 + rand()%15;
            (*navesec)[qntdnaves].dx = (evx - (*nave).x)/dist(evx, (*nave).x, evy, (*nave).y);
            (*navesec)[qntdnaves].dy = (evy - (*nave).y)/dist(evx, (*nave).x, evy, (*nave).y);
            (*navesec)[qntdnaves].cor = al_map_rgb(corAleatoria(), corAleatoria(), corAleatoria());
            (*navesec)[qntdnaves].x = evx;
            (*navesec)[qntdnaves].y = evy;
            qntdnaves++;
        }
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
        al_draw_text(fonte, al_map_rgb(0, 0, 0), SCREEN_W / 2,
                     (SCREEN_H - al_get_font_ascent(fonte)) / 2,
                     ALLEGRO_ALIGN_CENTRE, str);
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
        while(!feof(arq)){
            fgets(buf, MAX_TAM, arq);
            nomeaux = strtok(buf, "|");
            strcpy(nome[x],nomeaux);
            Rpont[x] = atoi(strtok(NULL, "|"));
            qntdjogadores++;
            x++;
        }

        for(i=0; i<=qntdjogadores; i++){
            for(j=0; j<=qntdjogadores; j++){
                if(Rpont[i]<Rpont[j]){
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
    menorTempo = Rpont[0];

    while(playing !=0){
        ALLEGRO_EVENT ev;
        /*inicia o temporizador*/
        al_start_timer(timer);

        al_wait_for_event(event_queue, &ev);

        if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            playing = 0;
            sair = true;
        }
        else if(ev.type == ALLEGRO_EVENT_TIMER) {
          tempo = al_get_timer_count(timer)/FPS;

          if(nave.x > SCREEN_W || nave.x < 0){
              nave.dx = -nave.dx;
          }
          if(naveinimiga.x > SCREEN_W || naveinimiga.x < 0){
              naveinimiga.dx = -naveinimiga.dx;
          }
          if(nave.y > SCREEN_H || nave.y < 0){
              nave.dy = -nave.dy;
          }
          if(naveinimiga.y > SCREEN_H || naveinimiga.y < 0){
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

                if(detectarColisaoInimigo(&naveinimiga, &nave)==1){
                    al_clear_to_color(al_map_rgb(255,255,255));
                    al_draw_textf(fonte, al_map_rgb(0, 0, 0), (SCREEN_W / 2), (SCREEN_H /8), ALLEGRO_ALIGN_CENTER, "Voce venceu!");
                    al_flip_display();
                    playing = 0;
                    Sleep(2000);
                }
                if(detectarColisaoInimigo(&nave, &naveinimiga)==1 || detectarColisaoPrincipal(&nave, &navesec)==0){
                    al_clear_to_color(al_map_rgb(255,255,255));
                    al_draw_textf(fonte, al_map_rgb(0, 0, 0), (SCREEN_W / 2), (SCREEN_H /8), ALLEGRO_ALIGN_CENTER, "Voce perdeu!");
                    al_draw_textf(fonte, al_map_rgb(0, 0, 0), (SCREEN_W / 2), (SCREEN_H /8)+40, ALLEGRO_ALIGN_CENTER,
                    "Fez %d pontos!", tempo);
                    al_flip_display();
                    playing = 0;
                    sair = true;
                    Sleep(2000);
                }
                else{
                    detectarColisaoPrincipal(&nave, &navesec);
                    detectarColisaoPrincipal(&naveinimiga, &navesec);
                    detectarColisaoSecundaria(&navesec);
                }
            }

            if(al_get_timer_count(timer)%500==0 && qntdnaves<=52){
                evx = 10 + rand() % (SCREEN_W - 20);
                evy = 10 + rand() % (SCREEN_H - 20);
                disparar(&nave, &navesec, evx, evy, 1);
            }

            al_flip_display();
            al_clear_to_color(BKG_COLOR);
            al_draw_textf(fonte, al_map_rgb(0, 0, 0), SCREEN_W / 25, SCREEN_H / 15, ALLEGRO_ALIGN_INTEGER, "%d", tempo);
        }

        else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {

            nave.dx = -((ev.mouse.x - nave.x)/dist(ev.mouse.x, nave.x, ev.mouse.y, nave.y));
            nave.dy = -((ev.mouse.y - nave.y)/dist(ev.mouse.x, nave.x, ev.mouse.y, nave.y));

            evx = 10 + rand() % (SCREEN_W - 20);
            evy = 10 + rand() % (SCREEN_H - 20);

            naveinimiga.dx = (evx - naveinimiga.x)/dist(evx, naveinimiga.x, evy, naveinimiga.y);
            naveinimiga.dy = (evy - naveinimiga.y)/dist(evy, naveinimiga.x, evy, naveinimiga.y);

            if((nave.raio>8 && qntdnaves<=52)){

                nave.dx+=1.0;
                nave.dy+=1.0;

                disparar(&nave, &navesec, ev.mouse.x, ev.mouse.y, 0);

                for(i=0; i<qntdnaves; i++){
                    desenhaDisparo(navesec[i]);
                }
            }
        }
    }

    if(sair == false){
        for(x=0, y=5; x<qntdjogadores; x++){
            if(tempo<Rpont[x]){
                y--;
            }
        }

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
                            sair = true;
                        }
                    }

                    if (evento.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
                    {
                        sair = true;
                    }
                }

                al_clear_to_color(al_map_rgb(255,255,255));

                if (!concluido)
                {
                    al_draw_textf(fonte, al_map_rgb(0, 0, 0), SCREEN_W / 2,
                                (SCREEN_H / 2 - al_get_font_ascent(fonte)) / 2,
                                ALLEGRO_ALIGN_CENTRE, "Agora voce faz parte do ranking com %d pontos! Nome:", tempo);
                }

                if(strlen(str)<17){
                    exibir_texto_centralizado();
                }
                al_flip_display();
            }
            qntdjogadores++;
            Rpont[qntdjogadores] = tempo;
            strcpy(nome[qntdjogadores], str);

            for(i=0; i<=qntdjogadores; i++){
                for(j=0; j<=qntdjogadores; j++){
                    if(Rpont[i]<Rpont[j]){
                        temp = Rpont[i];
                        Rpont[i] = Rpont[j];
                        Rpont[j] = temp;
                        strcpy(nomeaux, nome[i]);
                        strcpy(nome[i], nome[j]);
                        strcpy(nome[j], nomeaux);
                    }
                }
            }

             menorTempo = Rpont[0];

             arq = fopen("ranking.txt", "w");

            if(arq != NULL){
                for(x=0; x<qntdjogadores; x++){
                        if(strlen(nome[x])>0 && Rpont[x]>0){
                                fprintf(arq, "%s|%d|\n", nome[x], Rpont[x]);
                        }
                }
            }
            else{
                printf("Arquivo inexistente.");
            }
            fclose(arq);
        }

        else{
            al_clear_to_color(al_map_rgb(255,255,255));

            al_draw_textf(fonte, al_map_rgb(0, 0, 0), SCREEN_W / 2,
                                (SCREEN_H / 2 - al_get_font_ascent(fonte)) / 2,
                                ALLEGRO_ALIGN_CENTRE, "Que pena! Voce fez %d pontos e nao alcancou o lider do ranking com %d pontos!", tempo, menorTempo);
            al_flip_display();
        }
    }

        al_clear_to_color(al_map_rgb(255,255,255));

        al_draw_textf(fonte, al_map_rgb(0, 0, 0), (SCREEN_W / 2), (SCREEN_H /8), ALLEGRO_ALIGN_CENTER, "Ranking");

        for(x=0, y=0; x<qntdjogadores && x<5; x++){
            if(strlen(nome[x])>0 && Rpont[x]>0){
                al_draw_textf(fonte, al_map_rgb(0, 0, 0), (SCREEN_W / 2), (SCREEN_H /3)+y, ALLEGRO_ALIGN_CENTER, "[%s   %d]", nome[x], Rpont[x]);
                y+=40;
            }
            al_flip_display();
        }
        Sleep(4000);
}

int menu(){
    ALLEGRO_DISPLAY *janela = NULL;
    ALLEGRO_BITMAP *imagem = NULL;
    ALLEGRO_EVENT_QUEUE *fila = NULL;
    ALLEGRO_TIMER *temp = NULL;
    ALLEGRO_FONT *font = NULL, *font2 = NULL;
    int acao = 0, tempo;

    al_init();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_mouse();
    al_install_keyboard();
	al_init_primitives_addon();

    temp = al_create_timer(1.0 / FPS);
    fila = al_create_event_queue();
    font = al_load_ttf_font("comic.ttf", 75, 0);
    font2 = al_load_ttf_font("comic.ttf", 28, 0);

    al_set_window_title(janela, "Hrio");

	al_register_event_source(fila, al_get_mouse_event_source());
    al_register_event_source(fila, al_get_keyboard_event_source());
    al_register_event_source(fila, al_get_timer_event_source(temp));

    al_init_image_addon();
    janela = al_create_display(400, 600);
    imagem = al_load_bitmap("menu.jpg");

    al_draw_bitmap(imagem, 0, 0, 0);
    al_draw_textf(font, al_map_rgb(255, 255, 255), 280, 180, ALLEGRO_ALIGN_CENTER, "Hrio");
    al_draw_textf(font2, al_map_rgb(255, 255, 255), 250, 400, ALLEGRO_ALIGN_CENTER, "Clique para iniciar.");
    al_draw_textf(font2, al_map_rgb(255, 255, 255), 250, 430, ALLEGRO_ALIGN_CENTER, "Pressione Esc para sair.");

    al_flip_display();

    while(acao!=1){
        ALLEGRO_EVENT evento;
        al_start_timer(temp);
        al_wait_for_event(fila, &evento);

        if(evento.keyboard.keycode == ALLEGRO_KEY_ESCAPE){
            acao = 1;
            al_destroy_display(janela);
            al_destroy_bitmap(imagem);
            al_destroy_timer(temp);
            al_destroy_event_queue(fila);
            return 2;
        }
        else if(evento.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            acao = 1;
            al_destroy_display(janela);
            al_destroy_bitmap(imagem);
            al_destroy_timer(temp);
            al_destroy_event_queue(fila);
            return acao;
        }
    }
}

int main() {

    srand((unsigned) time(NULL));

    if(menu()==1){
        if(iniciar() != -1){
            jogo();
        }
        else{
            puts("Erro de Inicializacao.");
        }
    }

return 0;
}
