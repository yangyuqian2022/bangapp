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
#define N_ROWS 300000000

/**
 * @brief 
 * 
 */
typedef struct Row {
    int a;
    int b;
} Row;

typedef struct RangeSlice {
    Row left;
    Row right;
} RangeSlice;

/**
 * @brief Function used to generate large seeds for performance testing.
 * 
 * @param nrows Number of rows this function will generate and return.
 * @return Row* list of rows generated in this function.
 */
Row* generate_seed(int nrows)
{
    clock_t before = clock();
    
    time_t seconds;

    seconds = time(NULL);

    Row* rows = calloc(nrows, sizeof(Row));

    for (int i = 0; i < nrows; i++)
    {
        rows[i].a = i*N_BASE_A;
        rows[i].b = i*N_BASE_B;
    }

    clock_t after = clock();

    printf("---- Cost %ldus(%.2fms) to generate the seed. ----\n", 
            after-before, ((float)after-(float)before)/1000.0F);

    return rows;
}

/**
 * @brief Compare row1 and row2 for binary search/compare.
 * 
 * @param row1 left argument of the comparison.
 * @param row2 right argument of the comparison.
 * @return 0 when row1 == row2 if
 *         row1.a == row2.a and row1.b == row2.b
 * @return 1 when row1 > row2 if
 *         (row1.a > row2.a) || (row1.a >= row2.a && row1.b > row2.b)
 * @return 2 when row1 < row2 if
 *         (row1.a < row2.a) || (row1.a <= row2.a && row1.b < row2.b)
 * @return 3 for undefined, should print warning.
 */
uint8_t compare(Row row1, Row row2)
{
    if (row1.a == row1.a && row1.b == row2.b)
    {
        return 0;
    }

    if ((row1.a > row2.a) || (row1.a >= row2.a && row1.b > row2.b))
    {
        return 1;
    }

    if ((row1.a < row2.a) || (row1.a <= row2.a && row1.b < row2.b))
    {
        return 2;
    }

    return 3;
}

/**
 * @brief Search for left most position for binary search.
 * 
 * @param rows Rows
 * @param nrows number of rows
 * @return int index of the left most row
 */
int search_left_most_rowidx(const Row *rows, int nrows, 
                        int low, int high, Row range_right)
{
    int mid = low+(high-low)/2;

    if (low < 0 || high >= nrows || low >= high)
    {
        return low;
    }

    // <= 1 means, first Row is >= range_right
    if (compare(rows[low], range_right) <= 1)
    {
        return low;
    }

    do
    {
        // mid == range_right
        if (compare(rows[mid], range_right)==0)
        {
            // FIXME, cannot handle duplicate rows?
            // congras, found exact index for range_right
            return mid;
        }

        // mid > range_right
        if (compare(rows[mid], range_right) == 1)
        {
            return search_left_most_rowidx(rows, nrows, 
                low, mid-1, range_right);
        }

        // mid < range_right
        if (compare(rows[mid], range_right) == 2)
        {
            return search_left_most_rowidx(rows, nrows, 
                mid+1, high, range_right);
        }

    } while (mid > low);

    return low;
}

/**
 * @brief Search for right most postition for binary search.
 * 
 * @param rows rows
 * @param nrows number of rows
 * @return int index of the right most row
 */
int search_right_most_rowidx(const Row *rows, int nrows, 
                        int low, int high, Row range_left)
{
    int mid = low+(high-low)/2;

    if (low < 0 || high >= nrows || low >= high)
    {
        return high;
    }

    if (compare(rows[high], range_left) == 2)
    {
        return high-1;
    }

    do
    {
        // mid == range_right
        if (compare(rows[mid], range_left)==0)
        {
            // FIXME, cannot handle duplicate rows?
            // congras, found exact index for range_right
            return mid;
        }

        // mid > range_right
        if (compare(rows[mid], range_left) == 1)
        {
            return search_right_most_rowidx(rows, nrows, 
                low, mid-1, range_left);
        }

        // mid < range_right
        if (compare(rows[mid], range_left) == 2)
        {
            return search_right_most_rowidx(rows, nrows, 
                mid+1, high, range_left);
        }
    } while (mid < high);
    

    return high;
}

/**
 * @brief Scan given rows with specific handler.
 * 
 * @param rows Rows contain part or all dataset, see Row for more details.
 * @param nrows Number of input rows.
 * @param handle A callback function to let the scanner to 
 *               know whether a row is accepted, callback function
 *               returns true when the scanned row need to be kept,
 *               otherwise return false to remove a scanned row.
 *               You can pass a NULL handle to reject all rows.
 * @return How many rows that accepted by the processor
 */
int scan_process(const Row* rows, int nrows, uint8_t(*handle)(Row))
{
    clock_t before = clock();
    if (!rows)
    {
        return 0;
    }

    int accepted_cnt = 0;
    
    // binary search right most position for $range_left

    // binary search left most position for $range_right

    
    clock_t after = clock();

    printf("---- Cost: %ldus(%.2fms) to select %d rows- ----\n", 
            after-before, ((float)after-(float)before)/1000.0F, accepted_cnt);

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
uint8_t task2_handle(Row row)
{
    // a in (1000, 2000, 3000) and b between 10 and 50
    if ((row.a == 1000 || row.a == 2000 || row.a == 3000 ) && row.b >= 10 && row.b < 50)
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
void task2(const Row *rows, int nrows)
{
    scan_process(rows, nrows, task2_handle);
}

int main(void)
{
    // Generate dataset to verify given solutions.
    // Row* rows = generate_seed(N_ROWS);

    Row rows[] = {
        { 1000, 31 },
        { 1000, 72 },
        { 1500, 12 },
        { 1500, 34 },
        { 2000, 22 },
        { 2000, 33 },
    };

    Row range_left = {2000, 10};
    Row range_right = {2000, 50};
    printf("left bounder for (1000, 10) is %d\n", search_left_most_rowidx(rows, 6, 0, 5, range_left));
    printf("right bounder for (1000, 10) is %d\n", search_left_most_rowidx(rows, 6, 0, 5, range_right));

    // Execute task1
    task2(rows, N_ROWS);

    // Destroy generated dataset.
    // free(rows);
}
 