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
#define N_ROWS 3276800

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

    Row* rows = calloc(nrows, sizeof(Row));

    for (int i = 0; i < nrows; i++)
    {
        rows[i].a = N_BASE_A*(i+1);
        rows[i].b = N_BASE_B*(i+1);
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
int scan_process(const Row* rows, int nrows, bool(*handle)(Row))
{
    clock_t before = clock();
    if (!rows)
    {
        return 0;
    }

    int accepted_cnt = 0;

    for (int i = 0; i < nrows; i++)
    {
        if (handle && handle(rows[i]))
        {
            accepted_cnt++;
        }
    }

    clock_t after = clock();

    printf("---- Cost: %ldus(%.2fms) Total(%d) Found(%d) ----\n", 
            after-before, ((float)after-(float)before)/1000.0F, nrows , accepted_cnt);

    return accepted_cnt;
}

/**
 * @brief Handle Row according to task1.
 * 
 * @param row immutable object for all row.
 * @return 0 row is accepted
 * @return 1 move to left
 * @return 2 move to right
 */
bool task1_handle(Row row)
{
    // a in (1000, 2000, 3000) and b between 10 and 50
    if ((row.a == 1000 || row.a == 2000 || row.a == 3000) && row.b >= 10 && row.b < 50)
    {
        printf("%d,%d\n", row.a, row.b);
        return true;
    }

    return false;
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
    scan_process(rows, nrows, task1_handle);
}

int main(void)
{
    // Generate dataset to verify given solutions.
    Row* rows = generate_seed(N_ROWS);

    task1(rows, N_ROWS);

    // Destroy generated dataset.
    free(rows);
}
 