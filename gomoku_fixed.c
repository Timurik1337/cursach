#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#ifndef MAX_SIZE
#define MAX_SIZE 50
#endif

typedef struct {
    int r;
    int c;
} Move;

static int  N;
static int  W;
static char P;
static char A;
static char B[MAX_SIZE][MAX_SIZE];

static inline bool in_field(int r, int c) {
    return r >= 0 && r < N && c >= 0 && c < N;
}

static bool win(char ch) {
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            if (B[r][c] == ch) {
                int k;
                for (k = 0; k < W && c + k < N && B[r][c + k] == ch; k++);
                if (k == W) return true;
                for (k = 0; k < W && r + k < N && B[r + k][c] == ch; k++);
                if (k == W) return true;
                for (k = 0; k < W && r + k < N && c + k < N && B[r + k][c + k] == ch; k++);
                if (k == W) return true;
                for (k = 0; k < W && r + k < N && c - k >= 0 && B[r + k][c - k] == ch; k++);
                if (k == W) return true;
            }
    return false;
}

static bool board_full(void) {
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            if (B[r][c] == ' ')
                return false;
    return true;
}

static int score_position(void) {
    if (win(A)) return  1000;
    if (win(P)) return -1000;
    return 0;
}

static void gen_moves(int mv[][2], int *cnt) {
    *cnt = 0;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            if (B[r][c] == ' ') {
                bool near = false;
                for (int dr = -1; dr <= 1 && !near; dr++)
                    for (int dc = -1; dc <= 1 && !near; dc++) {
                        int nr = r + dr;
                        int nc = c + dc;
                        if (in_field(nr, nc) && B[nr][nc] != ' ')
                            near = true;
                    }
                if (near || *cnt == 0) {
                    mv[*cnt][0] = r;
                    mv[*cnt][1] = c;
                    (*cnt)++;
                    if (*cnt >= 200) return;
                }
            }
}

static int minimax(int depth, bool maximizing, int alpha, int beta) {
    int s = score_position();
    if (depth == 0 || abs(s) == 1000 || board_full())
        return s;

    int mv[200][2], mc;
    gen_moves(mv, &mc);

    if (maximizing) {
        int best = -INT_MAX;
        for (int i = 0; i < mc; i++) {
            int r = mv[i][0];
            int c = mv[i][1];
            B[r][c] = A;
            int v = minimax(depth - 1, false, alpha, beta);
            B[r][c] = ' ';
            if (v > best) best = v;
            if (best > alpha) alpha = best;
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = INT_MAX;
        for (int i = 0; i < mc; i++) {
            int r = mv[i][0];
            int c = mv[i][1];
            B[r][c] = P;
            int v = minimax(depth - 1, true, alpha, beta);
            B[r][c] = ' ';
            if (v < best) best = v;
            if (best < beta) beta = best;
            if (beta <= alpha) break;
        }
        return best;
    }
}

Move compute_best_move(int n, int w, const char brd[][MAX_SIZE],
                       char pl, char ai, int diff, char who) {
    N = n;
    W = w;
    P = pl;
    A = ai;
    for (int r = 0; r < N; r++)
        memcpy(B[r], brd[r], N);

    int mv[200][2], mc;
    gen_moves(mv, &mc);

    Move best = { -1, -1 };

    if (diff == 1) {
        int i = rand() % mc;
        best.r = mv[i][0];
        best.c = mv[i][1];
        return best;
    }

    int depth = (diff == 2) ? 1 : 3;
    int bestScore = -INT_MAX;

    if (who == P) {
        char tmp = A; A = P; P = tmp;
    }

    for (int i = 0; i < mc; i++) {
        int r = mv[i][0];
        int c = mv[i][1];
        B[r][c] = who;
        int s = minimax(depth - 1, who == A, -INT_MAX, INT_MAX);
        B[r][c] = ' ';
        if (s > bestScore) {
            bestScore = s;
            best.r = r;
            best.c = c;
        }
    }

    if (who == P) {
        char tmp = A; A = P; P = tmp;
    }

    if (best.r < 0) {
        int i = rand() % mc;
        best.r = mv[i][0];
        best.c = mv[i][1];
    }
    return best;
}
