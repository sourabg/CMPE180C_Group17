#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details. 
#include <pthread.h> 
#include <iostream>
#include <fstream>
using namespace std;

#define NUM_ROWS                9
#define NUM_COLS                9
#define NUM_GRIDS               9
#define NUM_SUB_GRID_ELEMENTS   9
#define NUM_SUB_GRID_COLS       3
#define NUM_SUB_GRID_ROWS       3
#define SUDUKO_PUZZLE_FILE      "puzzle.txt"

int suduko_puzzle[NUM_ROWS][NUM_COLS] = { 
{6, 2, 4, 5, 3, 9, 1, 8, 7},
{5, 1, 9, 7, 2, 8, 6, 3, 4},
{8, 3, 7, 6, 1, 4, 2, 9, 5},
{1, 4, 3, 8, 6, 5, 7, 2, 9},
{9, 5, 8, 2, 4, 7, 3, 6, 1},
{7, 6, 2, 3, 9, 1, 4, 5, 8},
{3, 7, 1, 9, 5, 6, 8, 4, 2},
{4, 9, 6, 1, 8, 2, 5, 7, 3},
{2, 8, 5, 4, 7, 3, 9, 1, 6}
};
int row_valid = 0;
int col_valid = 0;
int sub_grid_valid[NUM_GRIDS];

/*
  The following function validates all the rows of a suduko puzzle
*/
void *validate_rows_thread(void *arg) 
{
    int i, j, k;
    int validator[NUM_COLS];

    for(k = 0; k < NUM_COLS; k++) {
        validator[k] = -1;
    }

    for(i = 0; i < NUM_ROWS; i++) {
        for(j = 0; j < NUM_COLS; j++) {
            if(validator[suduko_puzzle[i][j] - 1] != -1) {
                printf("Found Invalid element in row %d at index %d\n", i, j);
                pthread_exit(NULL);
            } else {
                validator[suduko_puzzle[i][j] - 1] = suduko_puzzle[i][j];
            }
        }
        for(k = 0; k < NUM_COLS; k++) {
            validator[k] = -1;
        }
    }
    cout << "All rows valid" << endl;
    row_valid = 1;
    pthread_exit(NULL); 
} 

/*
  The following function validates all the columns of a suduko puzzle
*/
void *validate_cols_thread(void *arg) 
{   
    int i, j, k;
    int validator[NUM_ROWS];
    
    //printf("%s\n", __func__);
    
    for(k = 0; k < NUM_ROWS; k++) {
        validator[k] = -1;
    }

    for(i = 0; i < NUM_COLS; i++) {
        for(j = 0; j < NUM_ROWS; j++) {
            if(validator[suduko_puzzle[j][i] - 1] != -1) {
                printf("Found Invalid element in col %d at index %d\n", j, i);
                pthread_exit(NULL);
            } else {
                validator[suduko_puzzle[j][i] - 1] = suduko_puzzle[j][i];
            }
        }
        for(k = 0; k < NUM_ROWS; k++) {
            validator[k] = -1;
        }
    }
    cout << "All cols valid" << endl;
    col_valid = 1;
    pthread_exit(NULL);
}

/*
  The following function validates all the 9 subgrids of a suduko 
  puzzle
*/
void *validate_sub_grids_thread(void *arg) {
    int i, j, k;
    int validator[NUM_SUB_GRID_ELEMENTS];
    int sub_grid_num = * (int*) arg;
    int start_row_idx, start_col_idx;
    int element;

    //printf("%s: %d\n", __func__, sub_grid_num);

    for(k = 0; k < NUM_SUB_GRID_ELEMENTS; k++) {
        validator[k] = -1;
    }
    start_row_idx = (sub_grid_num / NUM_SUB_GRID_COLS) * NUM_SUB_GRID_COLS;
    start_col_idx = (sub_grid_num % NUM_SUB_GRID_ROWS) * NUM_SUB_GRID_ROWS;
    //printf("start row idx %d, start col idx %d\n", start_row_idx, start_col_idx);
    k = 0;

    for(i = 0; i < NUM_SUB_GRID_ROWS; i++) {
        for(j = 0; j < NUM_SUB_GRID_COLS; j++) {
            element = suduko_puzzle[start_row_idx + i][start_col_idx + j];
            if(validator[element - 1] != -1) {
                printf("Invalid element found in grid %d at offset[%d][%d]\n", sub_grid_num, i, j);
                pthread_exit(NULL);
            } else {
                validator[element - 1] = element;
            }
        }
    }
    cout << "Sub grid:" << sub_grid_num << " valid" << endl;
    sub_grid_valid[sub_grid_num] = 1;
    pthread_exit(NULL);
}

int main() 
{

    pthread_t row_validator_thread_id; 
    pthread_t col_validator_thread_id; 
    pthread_t sub_grid_validator_thread_id[NUM_GRIDS]; 
    int row_ret_val;
    int col_ret_val;
    int grid_ret_val[NUM_GRIDS];
    int sub_grid_invalid = 0;
    int i, j;
    int sub_grid_data[NUM_GRIDS];
    ifstream input;

    input.open(SUDUKO_PUZZLE_FILE);
    for(i = 0; i < NUM_ROWS; i++) {
        for(j = 0; j < NUM_COLS; j++) {
            input >> suduko_puzzle[i][j];
        }
    }
    cout << "Validating" << endl;
    for(i = 0; i < NUM_ROWS; i++) {
        for(j = 0; j < NUM_COLS; j++) {
            cout << suduko_puzzle[i][j] << " ";
        }
        cout << endl;
    }
    
    pthread_create(&row_validator_thread_id, NULL, validate_rows_thread, NULL); 
    pthread_create(&col_validator_thread_id, NULL, validate_cols_thread, NULL); 
    for(i = 0; i < NUM_GRIDS; i++) {
        sub_grid_data[i] = i;
        pthread_create(&sub_grid_validator_thread_id[i], NULL, validate_sub_grids_thread, &sub_grid_data[i]);
    }

    pthread_join(row_validator_thread_id, NULL); 
    pthread_join(col_validator_thread_id, NULL); 
    for(i = 0; i < NUM_GRIDS; i++) {
        pthread_join(sub_grid_validator_thread_id[i], NULL);
    }
    
    for (i = 0; i < NUM_GRIDS; i++) {
        if(!sub_grid_valid[i]) {
            sub_grid_invalid = 1;
            break;
        }
    }
    cout << "======Result==========" << endl;
    if(!row_valid || !col_valid || sub_grid_invalid) {
        printf("Invalid Puzzle\n");
    } else {
        printf("Valid Puzzle\n");
    }
    cout << "======================" << endl;
    
    return 0;
}
