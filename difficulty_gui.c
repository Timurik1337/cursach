/*
 * difficulty_gui.c – SDL2 dialog for choosing AI difficulty.
 * ---------------------------------------------------------
 * Shows description of each level, asks user to press 1‑3 + Enter,
 * then launches *уже собранную* игру.
 *
 * • На Windows сначала пытается запустить  "gomoku_fixed.exe",
 *   если файла нет – "gomoku.exe".
 * • На Linux/macOS аналогично: "./gomoku_fixed" → "./gomoku".
 *
 * Сборка (MSYS2 MinGW64):
 *   gcc -O2 difficulty_gui.c -o difficulty_gui.exe \
 *       -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>     /* _access */
#define access _access
#else
#include <unistd.h> /* access */
#endif

#define WINDOW_W 600
#define WINDOW_H 400
#define FONT_PATH "arial.ttf"

static SDL_Window   *win = NULL;
static SDL_Renderer *ren = NULL;
static TTF_Font     *font = NULL;

static char diffStr[4] = "";
static int  diffLen    = 0;

static void die(const char *msg){
    SDL_Log("%s: %s", msg, SDL_GetError());
    exit(1);
}

static void drawText(const char *txt,int x,int y){
    SDL_Color c={0,0,0,255};
    SDL_Surface *s = TTF_RenderUTF8_Blended(font, txt, c);
    SDL_Texture *t = SDL_CreateTextureFromSurface(ren,s);
    SDL_Rect dst={x,y,s->w,s->h};
    SDL_RenderCopy(ren,t,NULL,&dst);
    SDL_FreeSurface(s);
    SDL_DestroyTexture(t);
}

/* ---------- launch helper ------------------------------------------------ */
static void launchGame(int diff){
    char cmd[128];

#ifdef _WIN32
    /* сначала gomoku_fixed.exe, иначе gomoku.exe */
    if(access("gomoku_fixed.exe",0)==0){
        snprintf(cmd,128,"\"gomoku_fixed.exe\" %d",diff);
    } else if(access("gomoku.exe",0)==0){
        snprintf(cmd,128,"\"gomoku.exe\" %d",diff);
    } else {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Ошибка",
                                 "Не найден gomoku_fixed.exe или gomoku.exe в текущей папке.",
                                 win);
        return;
    }
#else
    if(access("./gomoku_fixed",X_OK)==0){
        snprintf(cmd,128,"./gomoku_fixed %d",diff);
    } else if(access("./gomoku",X_OK)==0){
        snprintf(cmd,128,"./gomoku %d",diff);
    } else {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
                                 "Error",
                                 "Cannot find ./gomoku_fixed or ./gomoku in current directory.",
                                 win);
        return;
    }
#endif
    system(cmd);
}

int main(int argc,char**argv){
    if(SDL_Init(SDL_INIT_VIDEO)!=0) die("SDL_Init");
    if(TTF_Init()!=0)               die("TTF_Init");

    win = SDL_CreateWindow("Выбор сложности",
                           SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                           WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if(!win) die("CreateWindow");
    ren = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!ren) die("CreateRenderer");

    font = TTF_OpenFont(FONT_PATH,18);
    if(!font) die("OpenFont");

    bool quit=false; SDL_Event e;
    while(!quit){
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) quit=true;

            else if(e.type==SDL_KEYDOWN){
                SDL_Keycode kc=e.key.keysym.sym;
                if(kc==SDLK_ESCAPE) quit=true;
                else if(kc==SDLK_BACKSPACE){ diffLen=0; diffStr[0]='\0'; }
                else if(kc>='1'&&kc<='3' && diffLen<1){
                    diffStr[0]=(char)kc; diffStr[1]='\0'; diffLen=1;
                }
                else if(kc==SDLK_RETURN && diffLen==1){
                    int diff=diffStr[0]-'0';
                    /* закрываем SDL прежде чем вызвать другую программу */
                    TTF_CloseFont(font); SDL_DestroyRenderer(ren); SDL_DestroyWindow(win);
                    TTF_Quit(); SDL_Quit();
                    launchGame(diff);
                    return 0;
                }
            }
        }

        SDL_SetRenderDrawColor(ren,240,255,255,255); SDL_RenderClear(ren);
        drawText("Уровни сложности:",40,30);
        drawText("1 – случайные ходы", 60,70);
        drawText("2 – Minimax глубина 1",60,100);
        drawText("3 – Minimax глубина 3",60,130);
        drawText("Введите 1‑3 и нажмите Enter:",40,190);

        SDL_Rect box={40,220,60,40};
        SDL_SetRenderDrawColor(ren,255,255,255,255); SDL_RenderFillRect(ren,&box);
        SDL_SetRenderDrawColor(ren,0,0,0,255); SDL_RenderDrawRect(ren,&box);
        drawText(diffStr[0]?diffStr:"_", box.x+20, box.y+8);

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit(); SDL_Quit();
    return 0;
}
