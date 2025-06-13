#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIN_W 900
#define WIN_H 700
#define MAX_SIZE 50
#define FONT_PATH "arial.ttf"

typedef struct { int r, c; } Move;
extern Move compute_best_move(int, int, const char[][MAX_SIZE], char, char, int, char);

enum Scene { SC_MENU, SC_SETUP, SC_GAME };
static enum Scene scene = SC_MENU;

static int boardSize = 15;
static int winLen    = 5;
static int diffLevel = 2;

static char playerChar = 'X';
static char aiChar     = 'O';

static char board[MAX_SIZE][MAX_SIZE];

/* geometry */
static int cellPx;
static int xOff, yOff;   /* renamed to avoid <math.h> y0() */

/* game state */
static bool gameEnd = false;
static char winner  = ' ';
static int  hintRow = -1, hintCol = -1, hintsLeft = 0;

/* SDL */
static SDL_Window   *gWindow = NULL;  /* renamed from win to avoid conflict */
static SDL_Renderer *gRen    = NULL;
static TTF_Font     *gFont   = NULL;

static void sdlDie(const char *msg) { SDL_Log("%s: %s", msg, SDL_GetError()); exit(1); }
static void drawText(const char *s,int x,int y) { SDL_Color c={0}; SDL_Surface *sf=TTF_RenderUTF8_Blended(gFont,s,c); SDL_Texture *tx=SDL_CreateTextureFromSurface(gRen,sf); SDL_Rect d={x,y,sf->w,sf->h}; SDL_RenderCopy(gRen,tx,NULL,&d); SDL_FreeSurface(sf); SDL_DestroyTexture(tx);} 

typedef struct { SDL_Rect r; const char *label; } Button;
static bool hover(Button *b,int mx,int my){ return mx>=b->r.x&&mx<b->r.x+b->r.w && my>=b->r.y&&my<b->r.y+b->r.h; }
static void drawButton(Button*b,bool hot){ SDL_SetRenderDrawColor(gRen,200,200,200,255); SDL_RenderFillRect(gRen,&b->r); SDL_SetRenderDrawColor(gRen,hot?255:0,hot?80:0,hot?80:0,255); SDL_RenderDrawRect(gRen,&b->r); drawText(b->label,b->r.x+8,b->r.y+4);} 

/* dynamic labels */
static char labDiff[24]  = "Сложность";
static char labSign[24]  = "Знак: X";
static char labHint[32]  = "Подсказка";

/* buttons */
static Button btnDiff  = {{WIN_W/2-80, WIN_H/2-140,160,40}, labDiff};
static Button btnSign  = {{WIN_W/2-80, WIN_H/2-90 ,160,40}, labSign};
static Button btnStart = {{WIN_W/2-80, WIN_H/2-40 ,160,40}, "Играть"};

static Button btnSizeM = {{WIN_W/2-140, WIN_H/2-80, 40,40}, "-"};
static Button btnSizeP = {{WIN_W/2+100, WIN_H/2-80, 40,40}, "+"};
static Button btnLenM  = {{WIN_W/2-140, WIN_H/2-20, 40,40}, "-"};
static Button btnLenP  = {{WIN_W/2+100, WIN_H/2-20, 40,40}, "+"};
static Button btnGo    = {{WIN_W/2-60 , WIN_H/2+40,120,40}, "Старт"};

static Button btnHint  = {{0,0,160,35}, labHint};

/* helpers */
static void clearBoard(void){ for(int r=0;r<boardSize;r++) memset(board[r],' ',boardSize); }
static bool inRange(int r,int c){ return r>=0&&r<boardSize&&c>=0&&c<boardSize; }
static bool checkWin(char ch){
    int need=winLen;
    for(int r=0;r<boardSize;r++)
        for(int c=0;c<boardSize;c++) if(board[r][c]==ch){
            int s;
            for(s=0;s<need && c+s<boardSize   && board[r][c+s]==ch; s++); if(s==need) return true;
            for(s=0;s<need && r+s<boardSize   && board[r+s][c]==ch; s++); if(s==need) return true;
            for(s=0;s<need && r+s<boardSize && c+s<boardSize && board[r+s][c+s]==ch; s++); if(s==need) return true;
            for(s=0;s<need && r+s<boardSize && c-s>=0       && board[r+s][c-s]==ch; s++); if(s==need) return true;
        }
    return false;
}
static bool boardFull(void){ for(int r=0;r<boardSize;r++) for(int c=0;c<boardSize;c++) if(board[r][c]==' ') return false; return true; }

static void calcGeom(void){ cellPx=(WIN_H-100)/boardSize; if(cellPx<10) cellPx=10; xOff=50; yOff=50; btnHint.r.x=xOff+boardSize*cellPx+30; btnHint.r.y=yOff; }
static void drawEllipse(int cx,int cy,int rx,int ry){ const int seg=64; double step=6.28318530718/seg; for(int i=0;i<seg;i++){ double a=i*step,b=a+step; SDL_RenderDrawLine(gRen,cx+rx*cos(a),cy+ry*sin(a),cx+rx*cos(b),cy+ry*sin(b)); }}
static void drawBoard(void){ SDL_SetRenderDrawColor(gRen,0,0,0,255);
    for(int i=0;i<=boardSize;i++){ int y=yOff+i*cellPx; SDL_RenderDrawLine(gRen,xOff,y,xOff+boardSize*cellPx,y); int x=xOff+i*cellPx; SDL_RenderDrawLine(gRen,x,yOff,x,yOff+boardSize*cellPx);} 
    for(int r=0;r<boardSize;r++) for(int c=0;c<boardSize;c++) if(board[r][c]!=' '){ int cx=xOff+c*cellPx+cellPx/2, cy=yOff+r*cellPx+cellPx/2; if(board[r][c]=='X'){ SDL_SetRenderDrawColor(gRen,200,0,0,255); SDL_RenderDrawLine(gRen,cx-cellPx/3,cy-cellPx/3,cx+cellPx/3,cy+cellPx/3); SDL_RenderDrawLine(gRen,cx+cellPx/3,cy-cellPx/3,cx-cellPx/3,cy+cellPx/3);} else { SDL_SetRenderDrawColor(gRen,0,0,200,255); drawEllipse(cx,cy,cellPx/3,cellPx/3);} }
    if(hintRow>=0){ SDL_SetRenderDrawColor(gRen,0,200,0,255); SDL_Rect rc={ xOff+hintCol*cellPx+1, yOff+hintRow*cellPx+1, cellPx-2, cellPx-2 }; SDL_RenderDrawRect(gRen,&rc);} }

static void playerClick(int mx,int my){ if(gameEnd) return; if(mx<xOff||my<yOff) return; int c=(mx-xOff)/cellPx, r=(my-yOff)/cellPx; if(!inRange(r,c)||board[r][c]!=' ') return; board[r][c]=playerChar; hintRow=hintCol=-1; if(checkWin(playerChar)){ gameEnd=true; winner=playerChar; return;} if(boardFull()){ gameEnd=true; return;} Move m=compute_best_move(boardSize,winLen,board,playerChar,aiChar,diffLevel,aiChar); board[m.r][m.c]=aiChar; if(checkWin(aiChar)){ gameEnd=true; winner=aiChar; } else if(boardFull()){ gameEnd=true; } }

/* scenes */
static void menuScene(SDL_Event *ev){ int mx,my; SDL_GetMouseState(&mx,&my); bool hDiff=hover(&btnDiff,mx,my), hSign=hover(&btnSign,mx,my), hStart=hover(&btnStart,mx,my);
    if(ev && ev->type==SDL_MOUSEBUTTONDOWN && ev->button.button==SDL_BUTTON_LEFT){ if(hDiff){ diffLevel = diffLevel%3 + 1; } if(hSign){ playerChar = (playerChar=='X')?'O':'X'; aiChar = (playerChar=='X')?'O':'X'; snprintf(labSign,sizeof(labSign),"Знак: %c",playerChar);} if(hStart) scene=SC_SETUP; }
    SDL_SetRenderDrawColor(gRen,230,230,250,255); SDL_RenderClear(gRen);
    drawButton(&btnDiff,hDiff); drawButton(&btnSign,hSign); drawButton(&btnStart,hStart);
    char num[4]; snprintf(num,4,"%d",diffLevel); drawText(num,btnDiff.r.x+btnDiff.r.w+10,btnDiff.r.y+8);
    drawText("Gomoku",WIN_W/2-40,80); }

static void setupScene(SDL_Event *ev){ int mx,my; SDL_GetMouseState(&mx,&my);
    bool hSm=hover(&btnSizeM,mx,my), hSp=hover(&btnSizeP,mx,my), hLm=hover(&btnLenM,mx,my), hLp=hover(&btnLenP,mx,my), hGo=hover(&btnGo,mx,my);
    if(ev && ev->type==SDL_MOUSEBUTTONDOWN && ev->button.button==SDL_BUTTON_LEFT){ if(hSm&&boardSize>5){ boardSize--; if(winLen>boardSize) winLen=boardSize;} if(hSp&&boardSize<MAX_SIZE) boardSize++; if(hLm&&winLen>3) winLen--; if(hLp&&winLen<boardSize) winLen++; if(hGo){ calcGeom(); clearBoard(); hintsLeft=boardSize-3; if(hintsLeft<0) hintsLeft=0; snprintf(labHint,sizeof(labHint),"Подсказка (%d)",hintsLeft); gameEnd=false; winner=' '; scene=SC_GAME;} }
    SDL_SetRenderDrawColor(gRen,245,245,245,255); SDL_RenderClear
