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
    #define FRAMES_correr 10
    #define FRAMES_ATACAR 3
    #define FRAMES_ESCALAR 4
    #define FRAMES_SALTAR 5
    #define FRAMES_CORRER_DISPARAR 10
    #define FRAMES_E_WALK 14
    #define FRAMES_E_ATK 7

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
        bool escaleras;
        int anim_frame;
        int timer_anim;
        int estado;
        int hp;
        int tiempo_invencible;
        bool herido;
        bool atacando;
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
        int direccion;
        int anim_frame;
        int timer_anim;
        float x_inicial;
        float rango_patrulla;
    } Enemigo;

    typedef struct {
        bool izquierda;
        bool derecha;
        bool salto;
        bool disparo;
        bool arriba;
        bool abajo;
    } Input;

    Imagen *spr_idle, *spr_plomazo, *spr_cubito, *bg;
    Imagen *spr_escalera, *spr_espinas, *spr_puerta, *spr_pasto, *spr_tierra, *spr_cajra;
    Imagen *spr_enemigo;
    Imagen *spr_idle_R, *spr_idle_I, *spr_plomazo, *spr_cubito, *bg;
    Imagen *spr_correr_D[FRAMES_correr], *spr_correr_I[FRAMES_correr];
    Imagen *spr_atacar_D[FRAMES_ATACAR], *spr_atacar_I[FRAMES_ATACAR];
    Imagen *spr_saltar_D[FRAMES_SALTAR], *spr_saltar_I[FRAMES_SALTAR];
    Imagen *spr_escalar[FRAMES_ESCALAR];
    Imagen *spr_correr_D_atacar[FRAMES_CORRER_DISPARAR], *spr_correr_I_atacar[FRAMES_CORRER_DISPARAR];
    Imagen *spr_e_walk_R[FRAMES_E_WALK], *spr_e_walk_L[FRAMES_E_WALK];
    Imagen *spr_e_atk_R[FRAMES_E_ATK], *spr_e_atk_L[FRAMES_E_ATK];

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
    void controlarAnimacion();
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
            "..................###### .......####........................H",
            "###################....########....########################H",

            "..........................................................H#",
            "..........................................................H#",
            "..........................................................H#",

            "P.........................................................H#",
            ".........................................E................H#",
            "..........................................................H#",
            "#####...^^^..######...^^^...#####.........................H#",
            "#...#........##...#.........#...#.........................H#",
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
            if (t_down == 90 || t_down == 122) {
                if (!mega.atacando) {
                    mega.atacando = true;
                    mega.anim_frame = 0;
                    mega.timer_anim = 0;
                    disparar();
                }
            }
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

    void controlarAnimacion() {
        mega.timer_anim++;
        int velocidad_anim = 6;
        int limite_frames = 0;

        if (mega.atacando) {
            if (mega.enPiso && mega.vx != 0) {
                mega.estado = 5;
                limite_frames = FRAMES_CORRER_DISPARAR;
                velocidad_anim = 5;
            }
            else {
                mega.estado = 4;
                limite_frames = FRAMES_ATACAR;
                velocidad_anim = 4;
            }
        }
        else if (mega.escaleras) {
            mega.estado = 3;
            limite_frames = FRAMES_ESCALAR;
            if (mega.vy == 0) mega.timer_anim = 0;
        }
        else if (!mega.enPiso) {
            mega.estado = 2;
            limite_frames = FRAMES_SALTAR;
        }
        else if (mega.vx != 0) {
            mega.estado = 1;
            limite_frames = FRAMES_correr;
        }
        else {
            mega.estado = 0;
            mega.anim_frame = 0;
        }

        if (limite_frames > 0 && mega.timer_anim >= velocidad_anim) {
            mega.anim_frame++;
            mega.timer_anim = 0;
            if (mega.anim_frame >= limite_frames) {
                mega.anim_frame = 0;
                if (mega.estado == 4 || mega.estado == 5) {
                    mega.atacando = false;
                }
            }
        }
    }

    void Fisicas() {
        if (controles.derecha && !controles.izquierda) {
            mega.vx = VELOCIDAD_MOV;
            mega.direccion = 1;
        } else if (controles.izquierda && !controles.derecha) {
            mega.vx = -VELOCIDAD_MOV;
            mega.direccion = -1;
        } else {
            mega.vx = 0;
        }

        if (mega.escaleras && controles.salto) {
            mega.escaleras = false;
            mega.vy = FUERZA_SALTO;
        }

        int tileX = (int)(mega.x + 25) / TILE_SIZE;
        int tileY = (int)(mega.y + 25) / TILE_SIZE;
        bool tocandoEscalera = false;

        if (tileX >= 0 && tileX < COLUMNAS && tileY >= 0 && tileY < FILAS) {
            if (mapa_nivel[tileY][tileX] == 'H') {
                tocandoEscalera = true;
            }
        }

        if (tocandoEscalera) {
            if (controles.arriba) { mega.escaleras = true; mega.vy = -4; mega.vx = 0; }
            else if (controles.abajo) { mega.escaleras = true; mega.vy = 4; mega.vx = 0; }
            else if (mega.escaleras) { mega.vy = 0; mega.vx = 0; }
        } else {
            mega.escaleras = false;
        }

        if (!mega.escaleras && controles.salto && mega.enPiso) {
            mega.vy = FUERZA_SALTO;
            mega.enPiso = false;
            controles.salto = false;
        }

        if (!mega.escaleras) mega.vy += GRAVEDAD;

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

        controlarAnimacion();

        int tileY_pies = (int)(mega.y + 45) / TILE_SIZE;
        if(tileX >= 0 && tileX < COLUMNAS && tileY_pies >= 0 && tileY_pies < FILAS) {
            if (mapa_nivel[tileY_pies][tileX] == '^') mega.hp = 0;
        }
    }
    void actualizarEnemigos() {
    for(int i = 0; i < MAX_ENEMIGOS; i++) {
        if (!enemigos[i].activo) continue;

        float dx = mega.x - enemigos[i].x;
        float dy = mega.y - enemigos[i].y;
        float distancia = sqrt(dx*dx + dy*dy);

        if (distancia < 300) {
            enemigos[i].estado_ia = 1;
            enemigos[i].direccion = (dx > 0) ? 1 : -1;
            enemigos[i].vx = 0;

            if (enemigos[i].cooldown_disparo <= 0) {
                enemigos[i].anim_frame = 0;
                enemigos[i].cooldown_disparo = 120;
            }

            if (enemigos[i].anim_frame == 3 && enemigos[i].timer_anim == 0) {
                enemigoDisparar(&enemigos[i]);
            }

        } else {
            enemigos[i].estado_ia = 0;
        }

        if (enemigos[i].cooldown_disparo > 0) {
            enemigos[i].cooldown_disparo--;
        }
        else {
            enemigos[i].estado_ia = 0;

            if (enemigos[i].direccion == 1) {
                enemigos[i].vx = 2.0;
                if (enemigos[i].x > enemigos[i].x_inicial + enemigos[i].rango_patrulla)
                    enemigos[i].direccion = -1;
            } else {
                enemigos[i].vx = -2.0;
                if (enemigos[i].x < enemigos[i].x_inicial - enemigos[i].rango_patrulla)
                    enemigos[i].direccion = 1;
            }
        }

        enemigos[i].timer_anim++;
        int vel_anim = (enemigos[i].estado_ia == 1) ? 6 : 4;
        int max_f = (enemigos[i].estado_ia == 1) ? FRAMES_E_ATK : FRAMES_E_WALK;

        if (enemigos[i].timer_anim >= vel_anim) {
            enemigos[i].anim_frame++;
            enemigos[i].timer_anim = 0;
            if (enemigos[i].anim_frame >= max_f) {
                enemigos[i].anim_frame = 0;
            }
        }

        enemigos[i].x += enemigos[i].vx;
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

        Imagen *sprite_jugador = NULL;

        if (mega.direccion == 1) {
            switch(mega.estado) {
                case 1: sprite_jugador = spr_correr_D[mega.anim_frame]; break;
                case 2: sprite_jugador = spr_saltar_D[mega.anim_frame]; break;
                case 3: sprite_jugador = spr_escalar[mega.anim_frame]; break;
                case 4: sprite_jugador = spr_atacar_D[mega.anim_frame]; break;
                case 5: sprite_jugador = spr_correr_D_atacar[mega.anim_frame]; break;
                default: sprite_jugador = spr_idle_R; break;
            }
        } else {
            switch(mega.estado) {
                case 1: sprite_jugador = spr_correr_I[mega.anim_frame]; break;
                case 2: sprite_jugador = spr_saltar_I[mega.anim_frame]; break;
                case 3: sprite_jugador = spr_escalar[mega.anim_frame]; break;
                case 4: sprite_jugador = spr_atacar_I[mega.anim_frame]; break;
                case 5: sprite_jugador = spr_correr_I_atacar[mega.anim_frame]; break;
                default: sprite_jugador = spr_idle_I; break;
            }
        }

        if (!mega.herido) {
            int dx = (int)mega.x - camara_x;
            int dy = (int)mega.y - camara_y;

            if(sprite_jugador) {
                ventana.muestraImagenEscalada(dx, dy, 50, 50, sprite_jugador);
            } else {
                ventana.muestraImagenEscalada(dx, dy, 50, 50, spr_idle_I);
            }
        }

        for(int i = 0; i < MAX_ENEMIGOS; i++) {
            if (!enemigos[i].activo) continue;

            int ex = (int)enemigos[i].x - camara_x;
            int ey = (int)enemigos[i].y - camara_y;

            if (ex > -100 && ex < ANCHO_PANTALA + 100) {
                Imagen *img_enemigo = NULL;

                if (enemigos[i].estado_ia == 0) {
                    img_enemigo = (enemigos[i].direccion == 1) ?
                        spr_e_walk_R[enemigos[i].anim_frame] :
                        spr_e_walk_L[enemigos[i].anim_frame];
                } else {
                    img_enemigo = (enemigos[i].direccion == 1) ?
                        spr_e_atk_R[enemigos[i].anim_frame] :
                        spr_e_atk_L[enemigos[i].anim_frame];
                }

                if (img_enemigo) {
                    ventana.muestraImagenEscalada(ex, ey, enemigos[i].w, enemigos[i].h, img_enemigo);
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

        for (int y = 0; y < FILAS; y++){
            for (int x = 0; x < COLUMNAS; x++){
                char celda = mapa_nivel[y][x];

                int posX = x * TILE_SIZE;
                int posY = y * TILE_SIZE;

                if (celda == '#' || celda == 'G' || celda == 'D'){
                    if (num_plataformas < MAX_PLATAFORMAS) {
                        plataformas[num_plataformas].x = posX;
                        plataformas[num_plataformas].y = posY;
                        plataformas[num_plataformas].w = TILE_SIZE;
                        plataformas[num_plataformas].h = TILE_SIZE;
                        num_plataformas++;
                    }
                }
                else if (celda == 'P'){
                    mega.x = posX;
                    mega.y = posY;
                }
                else if (celda == 'E') {
                    if (contador_enemigos < MAX_ENEMIGOS) {
                        enemigos[contador_enemigos].activo = true;
                        enemigos[contador_enemigos].x = (float)posX;
                        enemigos[contador_enemigos].y = (float)posY;

                        enemigos[contador_enemigos].x_inicial = (float)posX;
                        enemigos[contador_enemigos].rango_patrulla = 500.0f;
                        enemigos[contador_enemigos].direccion = -1;
                        enemigos[contador_enemigos].vx = -2.0f;
                        enemigos[contador_enemigos].vy = 0.0f;

                        enemigos[contador_enemigos].hp = 3;
                        enemigos[contador_enemigos].w = 40;
                        enemigos[contador_enemigos].h = 40;

                        enemigos[contador_enemigos].estado_ia = 0;
                        enemigos[contador_enemigos].anim_frame = 0;
                        enemigos[contador_enemigos].timer_anim = 0;
                        enemigos[contador_enemigos].cooldown_disparo = 60;

                        contador_enemigos++;
                    }
                }
            }
        }

        if (num_plataformas < MAX_PLATAFORMAS){
            plataformas[num_plataformas] = (Rect){-100, (FILAS * TILE_SIZE) + 100, COLUMNAS * TILE_SIZE, 50};
            num_plataformas++;
        }
    }

    void cargar() {
        char path[50], mask[50];
        for(int i=0; i<FRAMES_correr; i++){
            sprintf(path, "correr_%d.bmp", i); sprintf(mask, "correr_%d_mask.bmp", i);
            spr_correr_D[i] = ventana.creaImagenConMascara(path, mask);
        }

        for(int i=1; i<FRAMES_correr+1; i++){
            sprintf(path, "correr_1%d.bmp", i); sprintf(mask, "correr_1%d_mask.bmp", i);
            spr_correr_I[i-1] = ventana.creaImagenConMascara(path, mask);
        }

        for(int i=0; i<FRAMES_ATACAR; i++){
            sprintf(path, "atacar_%d.bmp", i); sprintf(mask, "atacar_%d_mask.bmp", i);
            spr_atacar_D[i] = ventana.creaImagenConMascara(path, mask);
            if (spr_atacar_D == NULL) {
                printf("No esta cargando el ataque");
            }
        }

        for(int i=1; i<FRAMES_ATACAR+1; i++){
            sprintf(path, "atacar_1%d.bmp", i); sprintf(mask, "atacar_1%d_mask.bmp", i);
            spr_atacar_I[i-1] = ventana.creaImagenConMascara(path, mask);
            if (spr_atacar_I == NULL) {
                printf("No esta cargando el ataque");
            }
        }

        for(int i=0; i<FRAMES_ESCALAR; i++){
            sprintf(path, "escalar_%d.bmp", i); sprintf(mask, "escalar_%d_mask.bmp", i);
            spr_escalar[i] = ventana.creaImagenConMascara(path, mask);
        }
        for(int i=0; i<FRAMES_SALTAR; i++){
            sprintf(path, "salto_%d.bmp", i); sprintf(mask, "salto_%d_mask.bmp", i);
            spr_saltar_D[i] = ventana.creaImagenConMascara(path, mask);
        }

        for(int i=1; i<FRAMES_SALTAR+1; i++){
            sprintf(path, "salto_1%d.bmp", i); sprintf(mask, "salto_1%d_mask.bmp", i);
            spr_saltar_I[i-1] = ventana.creaImagenConMascara(path, mask);
        }

        for (int i=0; i<FRAMES_CORRER_DISPARAR; i++){
            sprintf(path, "correr_ataque_%d.bmp", i); sprintf(mask, "correr_ataque_%d_mask.bmp", i);
            spr_correr_D_atacar[i] = ventana.creaImagenConMascara(path, mask);
        }

        for (int i=20; i<34; i++){
            sprintf(path, "metal_caminar_%d.bmp", i); sprintf(mask, "metal_caminar_%d_mask.bmp", i);
            spr_e_walk_R[i-20] = ventana.creaImagenConMascara(path, mask);
        }

        for (int i=0; i<FRAMES_E_WALK; i++){
            sprintf(path, "metal_caminar_%d.bmp", i); sprintf(mask, "metal_caminar_%d_mask.bmp", i);
            spr_e_walk_L[i] = ventana.creaImagenConMascara(path, mask);
        }


        for (int i=10; i<17; i++){
            sprintf(path, "metal_disparo_%d.bmp", i); sprintf(mask, "metal_disparo_%d_mask.bmp", i);
            spr_e_walk_R[i-10] = ventana.creaImagenConMascara(path, mask);
        }


        for (int i=0; i<FRAMES_E_ATK; i++){
            sprintf(path, "metal_disparo_%d.bmp", i); sprintf(mask, "metal_disparo_%d_mask.bmp", i);
            spr_e_walk_L[i] = ventana.creaImagenConMascara(path, mask);
        }


        /*for (int i=1; i<FRAMES_CORRER_DISPARAR+1; i++) {
            sprintf(path, "correr_ataque_1%d.bmp", i); sprintf(mask, "correr_ataque_1%d_mask.bmp", i);
            spr_correr_I_atacar[i] = ventana.creaImagenConMascara(path, mask);
        }*/

        spr_idle_R = ventana.creaImagenConMascara("megaman.bmp", "megaman_mask.bmp");
        spr_idle_I = ventana.creaImagenConMascara("megaman2.bmp", "megaman2_mask.bmp");
        spr_plomazo = ventana.creaImagenConMascara("plomazo.bmp", "mask_plomazo.bmp");
        spr_enemigo = ventana.creaImagenConMascara("enemy.bmp", "mask_enemy.bmp");
        bg = ventana.creaImagen("bg_future.bmp");
        spr_cubito = ventana.creaImagen("cubito.bmp");
        spr_pasto = ventana.creaImagen("pasto.bmp");
        spr_tierra = ventana.creaImagen("tierra.bmp");
        spr_espinas = ventana.creaImagen("espinitas.bmp");
        spr_escalera = ventana.creaImagen("escalera.bmp");
        spr_puerta = ventana.creaImagen("tile_puerta.bmp");
    }

    bool choco(Rect a, Rect b) {
        return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
    }