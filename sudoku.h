#ifndef SUDOKU_H
#define SUDOKU_H
#include<stdbool.h>
/* core of sudoku is 9x9x9 cube, number of floor means digit (1-9), 
 * 9x9 are rooms on the floor, every room has a state 
 * */
/* room state constants */
const int sudokuFree = 0;
const int sudokuFilled = 1;
const int sudokuForbidden = 2;
const int stackCellRCB = 0;
const int stackCellDS = 1;
/* Matrix is floor, floor has 9 flats, each flat is 3x3 cell (boxes) */
typedef struct Matrix{
  int value[9][9];
  int rowEmptyCells[9];
  int columnEmptyCells[9];
  int boxEmptyCells[9];
  bool rowFilled[9];
  bool columnFilled[9];
  bool boxFilled[9];
}Matrix;

typedef struct Sudoku{
  Matrix matrix[9];
  int emptyCells;
}Sudoku;

typedef struct RCBStackCell{
  int matrix, row, column;
  struct RCBStackCell * prev;
}RCBStackCell;

typedef struct RCBStack{
  RCBStackCell data[9][9][9];
  RCBStackCell * last;
}RCBStack;

typedef struct Coords{
  int m, r, c;
}Coords;

typedef struct DSStackCell{
  int forbiddenAmount, cellType;
  Coords filled;
  Coords forbidden[28];
}DSStackCell;

typedef struct DSStack{
  DSStackCell data[81];
  DSStackCell * stackPointer;
}DSStack;

bool RCBsearch(Sudoku * sudoku, RCBStack * rstack, DSStack * dsstack);
bool unrollDSStack(Sudoku * sudoku, int * matrix, int * row, int * column, DSStack * dsstack);
bool findEmpty(Sudoku * sudoku, int * matrix, int * i, int * j);
void initRCBStack(RCBStack * stack);
bool pushRCBStack(int matrix, int row, int column, RCBStack * stack);
bool popRCBStack(RCBStack * stack, Sudoku * sudoku, int * matrix, int * RCB);
bool setForbidden(Sudoku * sudoku, int a, int b, int c, RCBStack * stack, int cellType, DSStack * dsstack);
void fillCellInSudoku(Sudoku * sudoku, int a, int b, int c, RCBStack * stack, int dsstackCellType, DSStack * dsstack);
bool fillSudoku(int (*data)[][9], Sudoku * sudoku);
bool fillData(int (*data)[][9]);
void printSudoku(Sudoku * sudoku);
bool solve(Sudoku * sudoku);
void cleanPrintSudoku(Sudoku * sudoku);
#endif
