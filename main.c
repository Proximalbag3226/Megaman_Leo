#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "graficos.h"

#define ANCHO_PANTALA 800
#define ALTURA_PANTALA 600
#define GRAVEDAD 1
#define VELOCIDAD_MOV 8
#define FUERZA_SALTO -18
#define MAX_BALAS 5
#define MAX_BALAS_ENEMIGAS 20
#define MAX_ENEMIGOS 50
#define VIDA_JUGADOR_MAX 10
#define TILE_SIZE 50
#define FILAS 35
#define COLUMNAS 90
#define MAX_PLATAFORMAS 900

typedef struct {
    int x, y;
    int w, h;
} Rect;

typedef struct {
    float x, y;
    float vx, vy;
    int direccion;
    bool enPiso;
    bool disparando;
    int anim_frame;
    int estado;
    int hp;
    int tiempo_invencible;
    bool herido;
    bool escaleras;
} Jugador;

typedef struct {
    float x, y;
    float vx;
    bool activa;
    int direccion;
} Bala;

typedef struct {
    float x, y;
    float vx, vy;
    bool activa;
    int radio;
} BalaEnemigo;

typedef struct {
    float x, y;
    float vx, vy;
    int w, h;
    int hp;
    bool activo;
    Rect hitbox;
    int cooldown_disparo;
    int estado_ia;
} Enemigo;

typedef struct {
    bool izquierda;
    bool derecha;
    bool salto;
    bool disparo;
    bool arriba;
    bool abajo;
} Input;

Imagen *spr_idle, *spr_correr, *spr_saltar, *spr_plomazo, *spr_cubito, *bg;
Imagen *spr_escalera, *spr_espinas, *spr_puerta, *spr_pasto, *spr_tierra, *spr_cajra;
Imagen *spr_enemigo;

Jugador mega;
Bala balas[MAX_BALAS];
BalaEnemigo balas_enemigas[MAX_BALAS_ENEMIGAS];
Enemigo enemigos[MAX_ENEMIGOS];
Rect plataformas[MAX_PLATAFORMAS];
int num_plataformas = 0;
int camara_x = 0;
int camara_y = 0;
Input controles = {false, false, false, false, false, false};

void cargar();
void cargarNivel();
void Etrada_teclas();
void Fisicas();
void actualizarEnemigos();
void enemigoDisparar(Enemigo *e);
void actualizarBalasEnemigas();
void disparar();
void actualizarBalas();
void disparoEnemigos();
void verDaño();
void dibujarJuego();
bool choco(Rect a, Rect b);

int main() {
    printf("Iniciando juegiyto");
    ventana.tamanioVentana(ANCHO_PANTALA, ALTURA_PANTALA);
    ventana.tituloVentana("Mega Man Leo");

    cargar();
    cargarNivel();

    int tecla = TECLAS.NINGUNA;

    while (tecla != TECLAS.ESCAPE) {
        Etrada_teclas();
        if (ventana.teclaPresionada() == TECLAS.ESCAPE) break;

        Fisicas();
        actualizarBalas();

        actualizarEnemigos();
        actualizarBalasEnemigas();

        disparoEnemigos();
        verDaño();

        int target_cam_x = mega.x - (ANCHO_PANTALA / 2);
        camara_x = target_cam_x;

        if (camara_x < 0) camara_x = 0;
        if (camara_x > (COLUMNAS * TILE_SIZE) - ANCHO_PANTALA) camara_x = (COLUMNAS * TILE_SIZE) - ANCHO_PANTALA;

        int target_cam_y = mega.y - (ALTURA_PANTALA / 2);
        camara_y = target_cam_y;

        if (camara_y < 0) camara_y = 0;
        if (camara_y > (FILAS * TILE_SIZE) - ALTURA_PANTALA) camara_y = (FILAS * TILE_SIZE) - ALTURA_PANTALA;

        dibujarJuego();
        ventana.espera(16);
    }

    ventana.cierraVentana();
    return 0;
}

char mapa_nivel[FILAS][COLUMNAS + 1] = {
        "..........................................................DD",
        "..........................................................DD",
        ".......................................#####################",
        ".......................................#...................#",
        ".......................................#...................#",
        ".............................###########...................#",
        ".............................#.........#...^^^.......^^^...#",
        ".............................#.........#...................#",

        "............................H#.........#####################",
        "............................H#..............................",
        "............................H#..............................",
        "............................H#..............................",

        "H############################...............................",
        "H.......................................E...................",
        "H..........E................................................",
        "H...........................................................",
        "H#####...^^^....########...^^^....##########################",

        "H...........................................................",
        "H...........................................................",
        "H...........................................................",

        "###########################################################H",
        "...........................................................H",
        "......................E.......................E............H",
        "...........................................................H",
        "...................####........####........................H",
        "###################....########....########################H",

        "...........................................................H",
        "...........................................................H",
        "...........................................................H",

        "P..........................................................H",
        ".........................................E.................H",
        "...........................................................H",
        "#####...^^^...#####...^^^...#####..........................H",
        "#...#.........#...#.........#...#..........................H",
        "############################################################"
};

void Etrada_teclas() {
    int t_down = ventana.teclaPresionada();
    while (t_down != TECLAS.NINGUNA) {
        if (t_down == TECLAS.ARRIBA) controles.arriba = true;
        if (t_down == TECLAS.ABAJO) controles.abajo = true;
        if (t_down == TECLAS.IZQUIERDA) controles.izquierda = true;
        if (t_down == TECLAS.DERECHA)   controles.derecha = true;
        if (t_down == TECLAS.ESPACIO)   controles.salto = true;
        if (t_down == 90 || t_down == 122) disparar();
        t_down = ventana.teclaPresionada();
    }

    int t_up = ventana.teclaSoltada();
    while (t_up != TECLAS.NINGUNA) {
        if (t_up == TECLAS.ARRIBA) controles.arriba = false;
        if (t_up == TECLAS.ABAJO) controles.abajo = false;
        if (t_up == TECLAS.IZQUIERDA) controles.izquierda = false;
        if (t_up == TECLAS.DERECHA)   controles.derecha = false;
        if (t_up == TECLAS.ESPACIO)   controles.salto = false;
        t_up = ventana.teclaSoltada();
    }
}

void Fisicas() {
    if (controles.derecha && !controles.izquierda) {
        mega.vx = VELOCIDAD_MOV;
        mega.direccion = 1;
        mega.estado = 1;
    } else if (controles.izquierda && !controles.derecha) {
        mega.vx = -VELOCIDAD_MOV;
        mega.direccion = -1;
        mega.estado = 1;
    } else {
        mega.vx = 0;
        mega.estado = 0;
    }

    if (mega.escaleras && controles.salto) {
        mega.escaleras = false;
        mega.vy = -10;
    }

    bool tocandoEscalera = false;
    int tileX = (mega.x + 25) / TILE_SIZE;
    int tileY = (mega.y + 25) / TILE_SIZE;

    if (tileX >= 0 && tileX < COLUMNAS && tileY >= 0 && tileY < FILAS) {
        if (mapa_nivel[tileY][tileX] == 'H') tocandoEscalera = true;
    }

    if (tocandoEscalera) {
        if (controles.arriba) {
            mega.escaleras = true;
            mega.vy = -4;
            mega.vx = 0;
        } else if (controles.abajo) {
            mega.escaleras = true;
            mega.vy = 4;
            mega.vx = 0;
        } else if (mega.escaleras) {
            mega.vy = 0;
            mega.vx = 0;
        }

        if (tocandoEscalera) {
            if (controles.arriba) {
                mega.escaleras = true;
                mega.vy = -4;
                mega.vx = 0;
            } else if (controles.abajo) {
                mega.escaleras = true;
                mega.vy = 4;
                mega.vx = 0;
            } else if (mega.escaleras) {
                mega.vy = 0;
                mega.vx = 0;
            }
        } else {
            mega.escaleras = false;
        }

    } else {
        mega.escaleras = false;
    }

    if (!mega.escaleras && controles.salto && mega.enPiso) {
        mega.vy = FUERZA_SALTO;
        mega.enPiso = false;
        controles.salto = false;
    }

    if (!mega.escaleras) {
        mega.vy += GRAVEDAD;
    }

    mega.x += mega.vx;
    mega.y += mega.vy;
    mega.enPiso = false;

    Rect rMega = {(int)mega.x, (int)mega.y, 50, 50};
    for(int i=0; i<num_plataformas; i++) {
        if (choco(rMega, plataformas[i])) {

            if (mega.vy > 0 && (mega.y - mega.vy) + 50 <= plataformas[i].y + 20) {
                mega.y = plataformas[i].y - 50;
                mega.vy = 0;
                mega.enPiso = true;
                mega.escaleras = false;
            }
        }
    }

    if (mega.y > (FILAS * TILE_SIZE) + 100) {
        printf("Caiste al vacio!\n");
        cargarNivel();
    }

    if (!mega.enPiso && !mega.escaleras) mega.estado = 2;

    int tileY_pies = (mega.y + 45) / TILE_SIZE;
    if(tileX >= 0 && tileX < COLUMNAS && tileY_pies >= 0 && tileY_pies < FILAS) {
        if (mapa_nivel[tileY_pies][tileX] == '^') {
            printf("Te moriste picado apa\n");
            mega.hp = 0;
        }
    }
}

void actualizarEnemigos() {
    for(int i=0; i<MAX_ENEMIGOS; i++) {
        if (!enemigos[i].activo) continue;

        float dx = mega.x - enemigos[i].x;
        float dy = mega.y - enemigos[i].y;
        float distancia = fabs(dx);

        if (distancia < 400 && fabs(dy) < 200) {
            enemigos[i].estado_ia = 1;
            enemigos[i].vx = 0;

            if (enemigos[i].cooldown_disparo <= 0) {
                enemigoDisparar(&enemigos[i]);
                enemigos[i].cooldown_disparo = 90;
            }
        } else {
            enemigos[i].estado_ia = 0;
            if (enemigos[i].vx == 0) enemigos[i].vx = 2.0;
        }

        if (enemigos[i].cooldown_disparo > 0) enemigos[i].cooldown_disparo--;

        if (enemigos[i].estado_ia == 0) {
            enemigos[i].x += enemigos[i].vx;
        }

        enemigos[i].vy += GRAVEDAD;
        enemigos[i].y += enemigos[i].vy;

        Rect rEnemigo = {(int)enemigos[i].x, (int)enemigos[i].y, enemigos[i].w, enemigos[i].h};

        for(int j=0; j<num_plataformas; j++) {
            if (choco(rEnemigo, plataformas[j])) {
                if (enemigos[i].vy > 0 && (enemigos[i].y - enemigos[i].vy) + enemigos[i].h <= plataformas[j].y + 20) {
                    enemigos[i].y = plataformas[j].y - enemigos[i].h;
                    enemigos[i].vy = 0;
                } else {

                    enemigos[i].vx *= -1;
                    enemigos[i].x += enemigos[i].vx * 2;
                }
            }
        }
        enemigos[i].hitbox = rEnemigo;
    }
}

void enemigoDisparar(Enemigo *e) {
    for(int i=0; i<MAX_BALAS_ENEMIGAS; i++) {
        if (!balas_enemigas[i].activa) {
            balas_enemigas[i].activa = true;
            balas_enemigas[i].x = e->x + (e->w / 2);
            balas_enemigas[i].y = e->y + (e->h / 2);
            balas_enemigas[i].radio = 6;

            float dx = mega.x - e->x;
            float dy = (mega.y + 25) - e->y;
            float len = sqrt(dx*dx + dy*dy);

            if (len != 0) {
                balas_enemigas[i].vx = (dx / len) * 6.0;
                balas_enemigas[i].vy = (dy / len) * 6.0;
            } else {
                balas_enemigas[i].vx = -6.0;
                balas_enemigas[i].vy = 0;
            }
            break;
        }
    }
}

void actualizarBalasEnemigas() {
    for(int i=0; i<MAX_BALAS_ENEMIGAS; i++) {
        if (balas_enemigas[i].activa) {
            balas_enemigas[i].x += balas_enemigas[i].vx;
            balas_enemigas[i].y += balas_enemigas[i].vy;

            if (balas_enemigas[i].x < camara_x - 100 || balas_enemigas[i].x > camara_x + ANCHO_PANTALA + 100 ||
                balas_enemigas[i].y < camara_y - 100 || balas_enemigas[i].y > camara_y + ALTURA_PANTALA + 100) {
                balas_enemigas[i].activa = false;
            }
        }
    }
}

void disparar() {
    for(int i=0; i<MAX_BALAS; i++) {
        if (!balas[i].activa) {
            balas[i].activa = true;
            balas[i].x = mega.x + (mega.direccion == 1 ? 40 : -10);
            balas[i].y = mega.y + 15;
            balas[i].direccion = mega.direccion;
            balas[i].vx = 15 * mega.direccion;
            break;
        }
    }
}

void actualizarBalas() {
    for(int i=0; i<MAX_BALAS; i++) {
        if (balas[i].activa) {
            balas[i].x += balas[i].vx;
            if (balas[i].x > camara_x + ANCHO_PANTALA + 100 || balas[i].x < camara_x - 100) {
                balas[i].activa = false;
            }
        }
    }
}

void disparoEnemigos() {
    for(int i=0; i<MAX_BALAS; i++) {
        if (!balas[i].activa) continue;
        Rect rBala = {(int)balas[i].x, (int)balas[i].y, 10, 10};

        for(int j=0; j<MAX_ENEMIGOS; j++) {
            if (!enemigos[j].activo) continue;

            if (choco(rBala, enemigos[j].hitbox)) {
                balas[i].activa = false;
                enemigos[j].hp--;
                enemigos[j].x += (balas[i].vx > 0) ? 5 : -5;
                if (enemigos[j].hp <= 0) enemigos[j].activo = false;
                break;
            }
        }
    }
}

void verDaño() {
    if (mega.tiempo_invencible > 0) {
        mega.tiempo_invencible--;
        mega.herido = (mega.tiempo_invencible / 5) % 2;
        return;
    } else {
        mega.herido = false;
    }

    Rect rMega = {(int)mega.x, (int)mega.y, 50, 50};

    for(int i=0; i<MAX_ENEMIGOS; i++) {
        if (!enemigos[i].activo) continue;
        if (choco(rMega, enemigos[i].hitbox)) {
            mega.hp -= 2;
            mega.tiempo_invencible = 60;
            mega.vy = -10;

            if (enemigos[i].x > mega.x) mega.vx = -10; else mega.vx = 10;
        }
    }

    for(int i=0; i<MAX_BALAS_ENEMIGAS; i++) {
        if (!balas_enemigas[i].activa) continue;

        Rect rBalaE = {(int)balas_enemigas[i].x - 6, (int)balas_enemigas[i].y - 6, 12, 12};
        if (choco(rMega, rBalaE)) {
            mega.hp -= 1;
            mega.tiempo_invencible = 60;
            balas_enemigas[i].activa = false;
        }
    }

    if (mega.hp <= 0) {
        printf("Perdiste apa \n");
        cargarNivel();
    }
}

void dibujarJuego() {
    ventana.limpiaVentana();

    if(!bg) ventana.colorFondoRGB(20, 20, 40);
    else ventana.muestraImagenEscalada(0, 0, ANCHO_PANTALA, ALTURA_PANTALA, bg);

    int startCol = camara_x / TILE_SIZE;
    int endCol = (camara_x + ANCHO_PANTALA) / TILE_SIZE + 1;
    int startRow = camara_y / TILE_SIZE;
    int endRow = (camara_y + ALTURA_PANTALA) / TILE_SIZE + 1;

    if (startCol < 0) startCol = 0;
    if (endCol > COLUMNAS) endCol = COLUMNAS;
    if (startRow < 0) startRow = 0;
    if (endRow > FILAS) endRow = FILAS;

    for (int y = startRow; y < endRow; y++) {
        for (int x = startCol; x < endCol; x++) {
            char celda = mapa_nivel[y][x];

            if (celda == '.' || celda == 'P' || celda == 'E') continue;

            int drawX = (x * TILE_SIZE) - camara_x;
            int drawY = (y * TILE_SIZE) - camara_y;

            Imagen *img_a_usar = NULL;

            switch(celda) {
                case '#': img_a_usar = spr_cubito; break;
                case 'H': img_a_usar = spr_escalera; break;
                case '^': img_a_usar = spr_espinas; break;
                case 'D': img_a_usar = spr_puerta; break;
                case 'G': img_a_usar = spr_pasto; break;
            }

            if (img_a_usar)
                ventana.muestraImagenEscalada(drawX, drawY, TILE_SIZE, TILE_SIZE, img_a_usar);
            else
                ventana.rectanguloRelleno(drawX, drawY, drawX + TILE_SIZE, drawY + TILE_SIZE);
        }
    }

    ventana.color(COLORES.ROJO);
    for(int i=0; i<MAX_ENEMIGOS; i++) {
        if (enemigos[i].activo) {
            int ex = (int)enemigos[i].x - camara_x;
            int ey = (int)enemigos[i].y - camara_y;

            if (ex > -50 && ex < ANCHO_PANTALA && ey > -50 && ey < ALTURA_PANTALA) {
                if (spr_enemigo) ventana.muestraImagenEscalada(ex, ey, enemigos[i].w, enemigos[i].h, spr_enemigo);
                else ventana.rectanguloRelleno(ex, ey, ex + enemigos[i].w, ey + enemigos[i].h);
            }
        }
    }

    ventana.color(COLORES.MAGENTA);
    for(int i=0; i<MAX_BALAS_ENEMIGAS; i++) {
        if (balas_enemigas[i].activa) {
            int bx = (int)balas_enemigas[i].x - camara_x;
            int by = (int)balas_enemigas[i].y - camara_y;
            ventana.circuloRelleno(bx, by, balas_enemigas[i].radio);
        }
    }

    if (!mega.herido) {
        Imagen *sprite_actual = spr_idle;
        if (mega.estado == 1) sprite_actual = spr_correr;
        if (mega.estado == 2) sprite_actual = spr_saltar;
        if (mega.escaleras) sprite_actual = spr_escalera;

        int drawX = (int)mega.x - camara_x;
        int drawY = (int)mega.y - camara_y;

        if(sprite_actual) ventana.muestraImagenEscalada(drawX, drawY, 50, 50, sprite_actual);
        else {
            ventana.color(COLORES.CYAN);
            ventana.rectanguloRelleno(drawX, drawY, drawX + 50, drawY + 50);
            ventana.color(COLORES.BLANCO);
            if(mega.direccion == 1) ventana.rectanguloRelleno(drawX+40, drawY+10, drawX+50, drawY+20);
            else ventana.rectanguloRelleno(drawX, drawY+10, drawX+10, drawY+20);
        }
    }

    ventana.color(COLORES.AMARILLO);
    for(int i=0; i<MAX_BALAS; i++) {
        if (balas[i].activa) {
            int bx = (int)balas[i].x - camara_x;
            int by = (int)balas[i].y - camara_y;
            if(spr_plomazo) ventana.muestraImagen(bx, by, spr_plomazo);
            else ventana.circuloRelleno(bx, by, 5);
        }
    }

    ventana.color(COLORES.NEGRO);
    ventana.rectanguloRelleno(10, 10, 110, 30);
    ventana.color(COLORES.VERDE);
    if (mega.hp < 4) ventana.color(COLORES.ROJO);
    if (mega.hp > 0) ventana.rectanguloRelleno(12, 12, 12 + (mega.hp * 10), 28);

    ventana.actualizaVentana();
}

void cargarNivel() {
    num_plataformas = 0;

    for(int i=0; i<MAX_ENEMIGOS; i++) enemigos[i].activo = false;
    for(int i=0; i<MAX_BALAS; i++) balas[i].activa = false;
    for(int i=0; i<MAX_BALAS_ENEMIGAS; i++) balas_enemigas[i].activa = false;

    mega.vx = 0; mega.vy = 0;
    mega.direccion = 1;
    mega.enPiso = false;
    mega.escaleras = false;
    mega.hp = VIDA_JUGADOR_MAX;
    mega.tiempo_invencible = 0;
    camara_x = 0;
    camara_y = 0;

    int contador_enemigos = 0;

    for (int y = 0; y < FILAS; y++) {
        for (int x = 0; x < COLUMNAS; x++) {
            char celda = mapa_nivel[y][x];

            int posX = x * TILE_SIZE;
            int posY = y * TILE_SIZE;

            if (celda == '#' || celda == 'G' || celda == 'D') {
                if (num_plataformas < MAX_PLATAFORMAS) {
                    plataformas[num_plataformas].x = posX;
                    plataformas[num_plataformas].y = posY;
                    plataformas[num_plataformas].w = TILE_SIZE;
                    plataformas[num_plataformas].h = TILE_SIZE;
                    num_plataformas++;
                }
            }
            else if (celda == 'P') {
                mega.x = posX;
                mega.y = posY;
            }
            else if (celda == 'E') {
                if (contador_enemigos < MAX_ENEMIGOS) {
                    enemigos[contador_enemigos].activo = true;
                    enemigos[contador_enemigos].x = posX;
                    enemigos[contador_enemigos].y = posY;
                    enemigos[contador_enemigos].vx = -2.0;
                    enemigos[contador_enemigos].hp = 3;
                    enemigos[contador_enemigos].w = 40;
                    enemigos[contador_enemigos].h = 40;
                    enemigos[contador_enemigos].cooldown_disparo = 60;
                    contador_enemigos++;
                }
            }
        }
    }

    if (num_plataformas < MAX_PLATAFORMAS) {
        plataformas[num_plataformas] = (Rect){-100, (FILAS * TILE_SIZE) + 100, COLUMNAS * TILE_SIZE, 50};
        num_plataformas++;
    }
}

void cargar() {
    spr_idle = ventana.creaImagenConMascara("megaman_idle.bmp", "mask_idle.bmp");
    spr_correr  = ventana.creaImagenConMascara("megaman_correr.bmp", "mask_correr.bmp");
    spr_saltar = ventana.creaImagenConMascara("megaman_saltar.bmp", "mask_saltar.bmp");
    spr_plomazo = ventana.creaImagenConMascara("plomazo.bmp", "mask_plomazo.bmp");
    spr_cubito = ventana.creaImagen("cubito.bmp");
    spr_enemigo = ventana.creaImagenConMascara("enemy.bmp", "mask_enemy.bmp");
    bg = ventana.creaImagen("bg_future.bmp");
    spr_pasto = ventana.creaImagen("tile_pasto.bmp");
    spr_tierra  = ventana.creaImagen("tile_tierra.bmp");
    spr_cajra = ventana.creaImagen("tile_cajra.bmp");
    spr_espinas = ventana.creaImagen("tile_espinas.bmp");
    spr_escalera = ventana.creaImagen("tile_escalera.bmp");
    spr_puerta = ventana.creaImagen("tile_puerta.bmp");

    if (!spr_cubito) printf("Error al cargar las imagens, usando graficos simples");
}

bool choco(Rect a, Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}