#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BOARD_SIZE 20

char board[MAX_BOARD_SIZE][MAX_BOARD_SIZE];
int boardSize;
int neededInARow;
char userSymbol;
char compSymbol;

void initBoard() {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            board[i][j] = ' ';
        }
    }
}

void printBoard() {
    system("cls");
    printf("   ");
    for (int j = 0; j < boardSize; j++) {
        printf("%2d ", j + 1);
    }
    printf("\n");

    for (int i = 0; i < boardSize; i++) {
        printf("%2d ", i + 1);
        for (int j = 0; j < boardSize; j++) {
            printf(" %c ", board[i][j]);
        }
        printf("\n");
    }
}

char checkWin() {
    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            char current = board[i][j];
            if (current == ' ') continue;

            if (j + neededInARow - 1 < boardSize) {
                int count = 0;
                for (int k = 0; k < neededInARow; k++) {
                    if (board[i][j + k] == current) count++; else break;
                }
                if (count == neededInARow) return current;
            }

            if (i + neededInARow - 1 < boardSize) {
                int count = 0;
                for (int k = 0; k < neededInARow; k++) {
                    if (board[i + k][j] == current) count++; else break;
                }
                if (count == neededInARow) return current;
            }

            if (i + neededInARow - 1 < boardSize && j + neededInARow - 1 < boardSize) {
                int count = 0;
                for (int k = 0; k < neededInARow; k++) {
                    if (board[i + k][j + k] == current) count++; else break;
                }
                if (count == neededInARow) return current;
            }

            if (i + neededInARow - 1 < boardSize && j - (neededInARow - 1) >= 0) {
                int count = 0;
                for (int k = 0; k < neededInARow; k++) {
                    if (board[i + k][j - k] == current) count++; else break;
                }
                if (count == neededInARow) return current;
            }
        }
    }

    for (int i = 0; i < boardSize; i++) {
        for (int j = 0; j < boardSize; j++) {
            if (board[i][j] == ' ') return ' ';
        }
    }

    return 'D';
}

void userMove() {
    int row, col;
    while (1) {
        printf("Ваш ход (введите строку и столбец): ");
        scanf("%d %d", &row, &col);
        row--; col--;
        if (row >= 0 && row < boardSize && col >= 0 && col < boardSize && board[row][col] == ' ') {
            board[row][col] = userSymbol;
            break;
        } else {
            printf("Неверный ход. Попробуйте ещё раз.\n");
        }
    }
}

void computerMove() {
    int row, col;
    do {
        row = rand() % boardSize;
        col = rand() % boardSize;
    } while (board[row][col] != ' ');
    board[row][col] = compSymbol;
    printf("Ход компьютера: %d %d\n", row + 1, col + 1);
}

int main() {
    srand((unsigned)time(NULL));

    do {
        printf("Введите размер поля (от 3 до 20): ");
        scanf("%d", &boardSize);
    } while (boardSize < 3 || boardSize > 20);

    do {
        printf("Введите, сколько нужно подряд для победы (от 3 до 5): ");
        scanf("%d", &neededInARow);
    } while (neededInARow < 3 || neededInARow > 5);

    do {
        printf("Выберите, за кого будете играть (x/o): ");
        scanf(" %c", &userSymbol);
    } while (userSymbol != 'x' && userSymbol != 'o');

    compSymbol = (userSymbol == 'x') ? 'o' : 'x';

    initBoard();
    printBoard();

    printf("Вы играете за '%c'. Компьютер играет за '%c'.\n", userSymbol, compSymbol);
    printf("Для победы нужно собрать %d подряд.\n", neededInARow);

    while (1) {
        userMove();
        printBoard();
        char result = checkWin();
        if (result == userSymbol) {
            printf("Вы выиграли!\n");
            break;
        } else if (result == compSymbol) {
            printf("Компьютер выиграл!\n");
            break;
        } else if (result == 'D') {
            printf("Ничья!\n");
            break;
        }

        computerMove();
        printBoard();
        result = checkWin();
        if (result == userSymbol) {
            printf("Вы выиграли!\n");
            break;
        } else if (result == compSymbol) {
            printf("Компьютер выиграл!\n");
            break;
        } else if (result == 'D') {
            printf("Ничья!\n");
            break;
        }
    }

    return 0;
}
