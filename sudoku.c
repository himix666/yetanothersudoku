#include<string.h>
#include<stdio.h>
#include<stdbool.h>
#include"sudoku.h"
/* {{{ pushRCBStack() pushes cell into RCBStack */
bool pushRCBStack(int matrix, int row, int column, RCBStack * stack){
  /* printf("pushing to stack %d %d\n", matrix, RCB); */
  if(!stack)
    return false;
  if(!stack->last){
    stack->last = &(stack->data[matrix][row][column]);
    stack->data[matrix][row][column].prev = stack->last;
    return true;
  }
  else if(!stack->data[matrix][row][column].prev){
    stack->data[matrix][row][column].prev = stack->last;
    stack->last = &(stack->data[matrix][row][column]);
    return true;
  }
  else
    return false;
} /* }}} */
/* {{{ popRCBStack() pops cell from RCBStack */
bool popRCBStack(RCBStack * stack, Sudoku * sudoku, int * matrix, int * RCB){
  if(stack == NULL)
    return false;
  if(stack->last == NULL)
    return false;
  *matrix = stack->last->matrix;
  *RCB = stack->last->row;
  if(stack->last->prev == stack->last){
    stack->last->prev = NULL;
    stack->last = NULL;
    return true;
  }
  else{
    RCBStackCell * temp = stack->last;
    stack->last = stack->last->prev;
    temp->prev = NULL;
    return true;
  }
} /* }}} */
/* {{{ setForbidden() sets cell's state to forbidden, updates RBC and DS stacks */
bool setForbidden(Sudoku * sudoku, int a, int b, int c, RCBStack * rstack, int cellType, DSStack * dsstack){
  if(sudoku->matrix[a].value[b][c] == 0){
    sudoku->matrix[a].value[b][c] = sudokuForbidden;
    if(dsstack){
      dsstack->stackPointer->forbidden[dsstack->stackPointer->forbiddenAmount].m = a;
      dsstack->stackPointer->forbidden[dsstack->stackPointer->forbiddenAmount].r = b;
      dsstack->stackPointer->forbidden[dsstack->stackPointer->forbiddenAmount].c = c;
      dsstack->stackPointer->forbiddenAmount++;
    }
    if(sudoku->matrix[a].rowEmptyCells[b])
      sudoku->matrix[a].rowEmptyCells[b]--;
    if(rstack)
      for(int i = 0; i < 9; i++)
        if(sudoku->matrix[i].rowEmptyCells[b] == 1)
          pushRCBStack(i, b, c, rstack);
    if(sudoku->matrix[a].columnEmptyCells[c])
      sudoku->matrix[a].columnEmptyCells[c]--;
    if(rstack)
      if(sudoku->matrix[a].columnEmptyCells[c] == 1)
        pushRCBStack(a, b, c, rstack);
    if(sudoku->matrix[a].boxEmptyCells[c / 3 + b - b % 3])
      sudoku->matrix[a].boxEmptyCells[c / 3 + b - b % 3]--;
    if(rstack)
      if(sudoku->matrix[a].boxEmptyCells[c / 3 + b - b % 3] == 1)
        pushRCBStack(a, b, c , rstack);
 }
  return true;
} /* }}} */
/* {{{ fillCellInSudoku() fills cell in sudoku->matrix[a].value[b][c],
 * updates status of empty cells and filled rows etc
 * parameters must be valid, otherwise behavior is undefined
 */
void fillCellInSudoku(Sudoku * sudoku, int m, int r, int c, RCBStack * rstack, int cellType, DSStack * dsstack){
  /* setting value here so it would not get set to forbidden and put to rstack */
  sudoku->matrix[m].value[r][c] = sudokuFilled;
  sudoku->matrix[m].rowEmptyCells[r] = 0;
  sudoku->matrix[m].columnEmptyCells[c] = 0;
  sudoku->matrix[m].boxEmptyCells[c / 3 + r - r % 3] = 0;
  sudoku->emptyCells--;
  sudoku->matrix[m].rowFilled[r] = true;
  sudoku->matrix[m].columnFilled[c] = true;
  sudoku->matrix[m].boxFilled[c / 3 + r - r % 3] = true;
  if(dsstack){
    if(dsstack->stackPointer == NULL)
      dsstack->stackPointer = &(dsstack->data[0]);
    else
      dsstack->stackPointer++;
    dsstack->stackPointer->filled.m = m;
    dsstack->stackPointer->filled.r = r;
    dsstack->stackPointer->filled.c = c;
    dsstack->stackPointer->forbiddenAmount = 0;
    dsstack->stackPointer->cellType = cellType;
  }
  for(int i = 0;i < 9;i++){
    setForbidden(sudoku, m, r, i, rstack, cellType, dsstack);
    setForbidden(sudoku, m, i, c, rstack, cellType, dsstack);
  }
  for(int i = c - c % 3; i < c - c % 3 + 3; i++)
    for(int j = r - r % 3; j < r - r % 3 + 3; j++)
      setForbidden(sudoku, m, j, i, rstack, cellType, dsstack);
 for(int i = 0; i < 9; i++)
    if(i != m)
      setForbidden(sudoku, i, r, c, rstack, cellType, dsstack);
} /* }}} */
/* {{{ fillSudoku() fills sudoku's internals with data, j is column, i is row */ 
bool fillSudoku(int (*data)[][9], Sudoku * sudoku){
  memset(sudoku, 0 , sizeof((*sudoku)));
  sudoku->emptyCells = 81;
  for(int i = 0; i < 9; i++)
    for(int j = 0;j < 9; j++){
      sudoku->matrix[i].rowEmptyCells[j] = 9;
      sudoku->matrix[i].columnEmptyCells[j] = 9;
      sudoku->matrix[i].boxEmptyCells[j] = 9;
      sudoku->matrix[i].rowFilled[j] = false;
      sudoku->matrix[i].columnFilled[j] = false;
      sudoku->matrix[i].boxFilled[j] = false;
    }
  for(int i = 0; i < 9; i++)
    for(int j = 0; j < 9; j++)
      if((*data)[i][j] - '0')
        if(sudoku->matrix[(*data)[i][j] - '0' - 1].value[i][j])
          return false;
        else
          fillCellInSudoku(sudoku, (*data)[i][j] - '0' - 1, i, j, NULL, stackCellDS, NULL);
  return true;
} /* }}} */
/* {{{ fillData() extracts data from stdin into data-array */
bool fillData(int (*data)[][9]){
  int j = 0;
  char buf[512];
  while(fgets(buf,sizeof(buf) - 1, stdin)){
    for(int i = 0;buf[i] && i < sizeof(buf);i++){
      if((buf[i] >= '0') && (buf[i] <= '9')){
        (*data)[j / 9][j % 9] = buf[i];
        j++;
      }
      else if(buf[i] =='.'){
        (*data)[j / 9][j % 9] = '0';
        j++;
      }
      if(j == 81)
        return true;
    }
  }
  return false;
} /* }}} */
/* {{{ printSudoku() prints sudoku struct to the stdout */
void printSudoku(Sudoku * sudoku){
  FILE * dump = fopen("dumpsudoku.txt", "w");
  if(dump){
    for(int i = 0; i < 3; i++){
      for(int k = 0; k < 11; k++)
        for(int j = 0; j < 3; j++){
          if(k == 0){
            if(fprintf(dump,"%d:", i * 3 + j) < 0) return;}
          else
            fprintf(dump,"  ");
          for(int l = 0; l < 9;l++){
            if(k == 9){
              fprintf(dump,"%d", sudoku->matrix[i * 3 + j].columnEmptyCells[l]);
              continue;
            }
            else if(k == 10){
              fprintf(dump,"%d", sudoku->matrix[i * 3 + j].columnFilled[l]);
              continue;
            }
            fprintf(dump,"%d", sudoku->matrix[i * 3 + j].value[k][l]);
            if(l == 8){
              fprintf(dump," %d", sudoku->matrix[i * 3 + j].rowEmptyCells[k]);
              fprintf(dump," %d", sudoku->matrix[i * 3 + j].rowFilled[k]);
            }
          }  
          if(k < 3){
            fprintf(dump," ");
            for(int m = 0;m < 3; m++)
              fprintf(dump,"%d", sudoku->matrix[i * 3 + j].boxEmptyCells[k * 3 + m]);
            fprintf(dump," ");
            for(int m = 0;m < 3; m++)
              fprintf(dump,"%d", sudoku->matrix[i * 3 + j].boxFilled[k * 3 + m]);
            fprintf(dump," ");
          } 
          else if(k > 8)
            fprintf(dump,"             ");
          else
            fprintf(dump,"         ");
          if(j == 2)
            fprintf(dump,"\n");
        }
    }
    fprintf(dump, "empty cells:%d\n", sudoku->emptyCells);
    int value = 0;
    for(int i = 0;i < 9;i++){
      for(int j = 0;j < 9;j++){
        for(int k = 0; k < 9;k++)
          if(sudoku->matrix[k].value[i][j] == sudokuFilled){
            value = k + 1;
            break;
          }
        if(value)
          fprintf(dump, "%d", value);
        else
          fprintf(dump, "0");
        value = 0;
      }
    }
    fprintf(dump, "\n");
  }
  else
    printf("cannot dump\n");
  if(fclose(dump))
    printf("dumping error\n");
} /* }}} */
/* {{{ solve() solves sudoku, returns true if solved, false if failed
 * algorithm: if not solved seek for last in row, column or box and push it to
 * depth-search stack, add free cells found from checked marked as forbidden and
 * ready to be solved rows, columns and boxes to RCB-stack and use it instead of
 * counters. if not solved after RCB checks, try depth-search , if it fails, 
 * unroll depth-search stack till next valid value for empty space. Failing to
 * unroll and solution counter is 0 means data is bad and there is no solution.
 */
bool solve(Sudoku * sudoku){
  int solution = 0;
  DSStack dsstack;
  dsstack.stackPointer = NULL;
  RCBStack rstack;
  if(sudoku->emptyCells == 0){
    solution = 1;
    cleanPrintSudoku(sudoku);
    return true;
  }
  initRCBStack(&rstack);
  while(true){
    if(RCBsearch(sudoku, &rstack, &dsstack)){
      solution++;
      cleanPrintSudoku(sudoku);
    }
    /* {{{ Depth Search-block start */
    int matrix, row , column;
    if(!findEmpty(sudoku, &matrix, &row, &column)){
      if(!unrollDSStack(sudoku, &matrix, &row , &column, &dsstack)){
        if(!solution) 
          return false; /* no free spaces in sudoku and stack is empty - we got bad data */
        else
          return true;
      }
    }
    fillCellInSudoku(sudoku, matrix, row, column, NULL, stackCellDS, &dsstack);
    if(!sudoku->emptyCells){
      solution++;
      cleanPrintSudoku(sudoku);
    } /* }}} Depth Search-block end*/
  }
} /* }}} */
/* {{{ RCBsearch performs search for last items in row or column or box in sudoku and fills them */
bool RCBsearch(Sudoku * sudoku, RCBStack * rstack, DSStack * dsstack){
  bool update = false;
  int i, j, x, y; /* i , x = row ; j , y = column */
  for(i = 0; i < 9; i++){
    for(j = 0; j < 9; j++){
      if(rstack->last)
        popRCBStack(rstack, sudoku, &i, &j);
      if((sudoku->matrix[i].rowEmptyCells[j] == 1) &&
          (sudoku->matrix[i].rowFilled[j] == false)){
        update = true;
        y = j;
        for(int k = 0; k < 9; k++){
          if(sudoku->matrix[i].value[j][k] == sudokuFree){
            x = k;
            break;
          }
        }
        fillCellInSudoku(sudoku, i, y, x, rstack, stackCellRCB, dsstack);
        if(!sudoku->emptyCells)
          goto foundSolution;
      }
      if((sudoku->matrix[i].columnEmptyCells[j] == 1) &&
          (sudoku->matrix[i].columnFilled[j] == false)){
        update = true;
        x = j;
        for(int k = 0;k < 9;k++){
          if(sudoku->matrix[i].value[k][j] == sudokuFree){
            y = k;
            break;
          }
        }
        fillCellInSudoku(sudoku, i, y, x, rstack, stackCellRCB, dsstack);
        if(!sudoku->emptyCells)
          goto foundSolution;
      }
      if((sudoku->matrix[i].boxEmptyCells[j] == 1) &&
          (sudoku->matrix[i].boxFilled[j] == false)){
        update = true;
        for(int k = (j % 3) * 3;k < (j % 3) * 3 + 3;k++)/* k = column */
          for(int l = j - j % 3;l < j - j % 3 + 3;l++){ /* l = row */
            if(sudoku->matrix[i].value[l][k] == sudokuFree){
              y = l;
              x = k;
              goto fillCellInSudokuUsingBoxData;
            }
          }
fillCellInSudokuUsingBoxData:
        fillCellInSudoku(sudoku, i, y, x, rstack, stackCellRCB, dsstack);
        if(!sudoku->emptyCells)
          goto foundSolution;
      }
      if(update){
        update = false;
        i = 0;
      }
    }
  }     
  return false;
foundSolution:
  if(rstack->last) /* clean RCB-stack */
    popRCBStack(rstack, sudoku, &i, &j);
  return true;
} /* }}} */
/* {{{ findEmpty() finds next empty cell in sudoku */
bool findEmpty(Sudoku * sudoku, int * matrix, int * row, int * column){
  for(int i = 0; i < 9; i++)
    for(int j = 0; j < 9; j++)
      for(int k = 0; k < 9; k++)
        if(!sudoku->matrix[i].value[j][k]){
          *matrix = i;
          *row = j;
          *column = k;
          return true;
        }
  return false;
} /* }}} */
/* {{{ unrollDSStack() unrolls stack
 * if successful - updates matrix , row and column */
bool unrollDSStack(Sudoku * sudoku, int * matrix, int * row, int * column, DSStack * dsstack){
  while(dsstack->stackPointer){
    DSStackCell * sp = dsstack->stackPointer;
    sudoku->matrix[sp->filled.m].value[sp->filled.r][sp->filled.c] = sudokuFree;
    sudoku->matrix[sp->filled.m].rowEmptyCells[sp->filled.r] = 1;
    sudoku->matrix[sp->filled.m].columnEmptyCells[sp->filled.c] = 1;
    sudoku->matrix[sp->filled.m].boxEmptyCells[sp->filled.c / 3 + sp->filled.r - sp->filled.r % 3] = 1;
    sudoku->emptyCells++;
    sudoku->matrix[sp->filled.m].rowFilled[sp->filled.r] = false;
    sudoku->matrix[sp->filled.m].columnFilled[sp->filled.c] = false;
    sudoku->matrix[sp->filled.m].boxFilled[sp->filled.c / 3 + sp->filled.r - sp->filled.r % 3] = false;
    for(int i = 0; i < sp->forbiddenAmount; i++){
      sudoku->matrix[sp->forbidden[i].m].value[sp->forbidden[i].r][sp->forbidden[i].c] = sudokuFree;
      sudoku->matrix[sp->forbidden[i].m].rowEmptyCells[sp->forbidden[i].r]++;
      sudoku->matrix[sp->forbidden[i].m].columnEmptyCells[sp->forbidden[i].c]++;
      sudoku->matrix[sp->forbidden[i].m].boxEmptyCells[sp->forbidden[i].c / 3 + sp->forbidden[i].r - sp->forbidden[i].r % 3]++;
    }
    if(dsstack->stackPointer == &(dsstack->data[0]))
      dsstack->stackPointer = NULL;
    else
      dsstack->stackPointer--;
    if(sp->cellType){ /* DS-type cell, try to find next empty value */
      for(int i = sp->filled.m + 1; i < 9; i++)
        if(!(sudoku->matrix[i].value[sp->filled.r][sp->filled.c])){
          *matrix = i;
          *row = sp->filled.r;
          *column = sp->filled.c;
          return true;
        }
    }
  }
  return false;
} /* }}} */
/* {{{ cleanPrintSudoku() prints sudoku's value's in human-readable format */
void cleanPrintSudoku(Sudoku * sudoku){
  int value = 0;
  for(int i = 0;i < 9;i++){
    for(int j = 0;j < 9;j++){
      for(int k = 0; k < 9;k++)
        if(sudoku->matrix[k].value[i][j] == sudokuFilled){
          value = k + 1;
          break;
        }
      if(value)
        printf("%d", value);
      else
        printf("0");
      value = 0;
    }
  }
  printf("\n");
} /* }}} */
/* {{{ initRCBStack() initializes RCB stack */
void initRCBStack(RCBStack * stack){
  for(int i = 0; i < 9; i++)
    for(int j = 0; j < 9; j++)
      for(int k = 0; k < 9; k++){
      stack->data[i][j][k].matrix = i;
      stack->data[i][j][k].row = j;
      stack->data[i][j][k].column = k;
      stack->data[i][j][k].prev = NULL;
    }
  stack->last = NULL;
} /* }}} */
int main(){
  int data[9][9];
  Sudoku sudoku;
  if(!fillData(&data)){
    printf("Data error, not enough data\n");
    return 0;
  }
  if(!fillSudoku(&data, &sudoku))
    printf("Error sudoku init\n");
  if(!solve(&sudoku)){
    printf("Failed to solve.\n");
    cleanPrintSudoku(&sudoku);
  }
  return 0;
}
/* vim: set tabstop=2 shiftwidth=2 softtabstop=2 noexpandtab: */
