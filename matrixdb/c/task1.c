#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/* 
 * When generate a Row,
 * column `a` is generated
 * with $N_BASE_A + a random number calculated 
 * from current timestamp,
 * column `b` is generated
 * with $N_BASE_B + a random number calculated
 * from current timestamp.
 */
#define N_BASE_A 1000
#define N_BASE_B 10
// Number of rows that generated for testing.
#define N_ROWS 10

/**
 * @brief 
 * 
 */
typedef struct Row {
    int a;
    int b;
} Row;

/**
 * @brief Function used to generate large seeds for performance testing.
 * 
 * @param nrows Number of rows this function will generate and return.
 * @return Row* list of rows generated in this function.
 */
Row* generate_seed(int nrows)
{
    time_t seconds;

    seconds = time(NULL);

    Row* rows = calloc(nrows, sizeof(Row));

    for (int i = 0; i < nrows; i++)
    {
        rows[i].a = (seconds%N_ROWS)*(N_BASE_A+i);
        rows[i].b = (seconds%N_ROWS)*(N_BASE_B+i);
    }

    return rows;
}

/**
 * @brief Scan given rows with specific handler.
 * 
 * @param rows Rows contain part or all dataset, see Row for more details.
 * @param nrows Number of input rows.
 * @param accepted arrary of indices of all accepted rows.
 * @param handle A callback function to let the scanner to 
 *               know whether a row is accepted, callback function
 *               returns true when the scanned row need to be kept,
 *               otherwise return false to remove a scanned row.
 *               You can pass a NULL handle to reject all rows.
 * @return How many rows that accepted by the processor
 */
int scan_process(Row* rows, int nrows, int accepted[], bool(*handle)(Row))
{
    if (!rows)
    {
        return NULL;
    }

    for (int i = 0; i < nrows; i++)
    {
        if (handle && handle(rows[i]))
        {
            accepted[];
        }
    }

    return NULL;
}

/**
 * @brief Task 1. Find out all the rows that sastify below conditions:
 *                ((b >= 10 && b < 50) && (a == 1000 || a == 2000 || a == 3000))
 *        
 *        Print them to the terminal, one row per line, for example:
 *           1000,20
 *           1000,23
 *           2000,16
 *
 * @param rows The total number of rows.
 * @param nrows The rows, for example rows[0] is the first row.
 */
void task1(const Row *rows, int nrows)
{

}

int main(void)
{
    // Generate dataset to verify given solutions.
    Row* rows = generate_seed(N_ROWS);

    // Destroy generated dataset.
    free(rows);
}
 