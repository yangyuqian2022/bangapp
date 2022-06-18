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
#define N_ROWS 4000000

/**
 * @brief 
 * 
 */
typedef struct Row {
    int a;
    int b;
} Row;

// range slice for config the [left, right)
typedef struct RangeSlice {
    Row left;
    Row right;
} RangeSlice;

typedef struct Node Node;
// Linked list for ordering b column
typedef struct Node {
    Row   value;
    Node* next;
} Node;

RangeSlice range_slices[] = {
    {{1000,10}, {99000,50}},
};

/**
 * @brief MapEntry value_b with insertion point in the resultset.
 * 
 */
typedef struct MapEntry {
    Node* point; // Pointer to last insertion point with column b == $val_b
} MapEntry;

MapEntry* rmap = NULL;

Node* ordered_rows = NULL;
Node* ordered_rows_tail = NULL;
int ordered_rows_count = 0;

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
        //printf("DEBUG: row = {%d,%d}\n", rows[i].a, rows[i].b);
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
    if (row1.a == row2.a && row1.b == row2.b)
    {
        // printf("DEBUG: (%d,%d)==(%d,%d)\n", row1.a, row1.b, row2.a, row2.b);
        return 0;
    }

    if ((row1.a > row2.a) || (row1.a >= row2.a && row1.b > row2.b))
    {
        // printf("DEBUG: (%d,%d)>(%d,%d)\n", row1.a, row1.b, row2.a, row2.b);
        return 1;
    }

    if ((row1.a < row2.a) || (row1.a <= row2.a && row1.b < row2.b))
    {
        // printf("DEBUG: (%d,%d)<(%d,%d)\n", row1.a, row1.b, row2.a, row2.b);
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

    // printf("DEBUG: right_index(%d,%d) nrows(%d) low(%d) mid(%d) high(%d)\n", range_right.a, range_right.b, nrows, low, mid, high);

    // not found
    if (low == high && low == (nrows-1) && 
            compare(rows[high], range_right) != 0)
    {
        return nrows;
    }

    if (high >= nrows || low >= high)
    {
        return low;
    }

    /*
    if (low == high && low == nrows - 1 && 
            compare(rows[nrows-1], range_right) != 0)
    {
        return nrows;
    }
    */

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

    // printf("DEBUG: left_index(%d,%d) nrows(%d) low(%d) mid(%d) high(%d)\n", range_left.a, range_left.b, nrows, low, mid, high);

    // not found
    if (low == high && low == (nrows-1) && compare(rows[high], range_left) != 0)
    {
        // printf("DEBUG: left_index(%d,%d) not found nrows(%d) low(%d) mid(%d) high(%d)\n", range_left.a, range_left.b, nrows, low, mid, high);
        return 0;
    }

    if (low > high)
    {
        // not found
        return 0;
    }

    while(mid < high)
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
            // printf("DEBUG: (%d,%d)>left_index(%d,%d) nrows(%d) low(%d) mid(%d) high(%d)\n", 
            //    rows[mid].a, rows[mid].b, range_left.a, range_left.b, nrows, low, mid, high);
            return search_right_most_rowidx(rows, nrows, 
                low, mid-1, range_left);
        }

        // mid < range_right
        if (compare(rows[mid], range_left) == 2)
        {
            // printf("DEBUG: (%d,%d)<left_index(%d,%d) nrows(%d) low(%d) mid(%d) high(%d)\n", 
            //    rows[mid].a, rows[mid].b, range_left.a, range_left.b, nrows, low, mid, high);
            return search_right_most_rowidx(rows, nrows, 
                mid+1, high, range_left);
        }
    }
    

    return high;
}

void print_ordered_rows()
{
    // printf("DEBUG: printing ordered rows (%d,%d)...\n", 
                // ordered_rows->value.a, ordered_rows->value.b);
    // Node* parent = NULL;
    Node* cur    = ordered_rows;
    while(cur)
    {
        printf("%d,%d\n", cur->value.a, cur->value.b);

        cur = cur->next;
        /*
        parent = cur;
        parent->next = NULL;
        parent = NULL;
        free(parent);
        */
    }
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

    int n_slices = sizeof(range_slices)/sizeof(RangeSlice);
    int finished_idx = -1;
    for (int i = 0; i< n_slices; i++)
    {
        RangeSlice slice = range_slices[i];
        Row range_left = slice.left;
        Row range_right = slice.right;
        // --right most-->|left_index  right_index|<--left most---
        int right_idx = search_left_most_rowidx(rows, nrows, 0, nrows-1, range_right);
        int left_idx = search_right_most_rowidx(rows, nrows, 0, nrows-1, range_left);
        right_idx = right_idx < 0 ? 0 : right_idx;
        left_idx = left_idx < 0 ? 0 : left_idx;
        
        /*
        printf("DEBUG: (%d,%d)[%d]->(%d,%d)[%d]  finished(%d)\n", 
                range_left.a, range_left.b, left_idx, 
                range_right.a, range_right.b, right_idx, finished_idx);
        */
       
        for (int j = left_idx; j <= right_idx && j>=0 && j<nrows; j++)
        {
            if (handle && j > finished_idx && handle(rows[j]))
            {
                finished_idx = j;
                accepted_cnt++;
            }
            // printf("DEBUG: handled (%d,%d)[%d] finished_at(%d)\n", rows[j].a, rows[j].b, j, finished_idx);
        }
    }

    print_ordered_rows();

    clock_t after = clock();

    printf("---- Cost: %ldus(%.2fms) Total(%d) Found(%d) ----\n", 
            after-before, ((float)after-(float)before)/1000.0F, nrows , accepted_cnt);

    return accepted_cnt;
}

void ordered_insert(Row row)
{
    ordered_rows_count++;
    Node *node = (Node *)malloc(sizeof(Node));
    node->value = row;
    node->next = NULL;

    if (rmap[row.b].point)
    {
        // already know the insertion point,
        // do the insertion and update the insertion point.
        Node* ins_child = rmap[row.b].point->next;
        // append incoming node to corresponding insertion point.
        node->next = ins_child;
        rmap[row.b].point->next = node;
        // update the insertion point accordingly.
        rmap[row.b].point = node;
    }

    if (!ordered_rows)
    {
        ordered_rows = node;
        ordered_rows_tail = node;

        // printf("DEBUG: created root(%d,%d)\n", row.a,row.b);
    }
    else
    {
        Node* cur = ordered_rows;
        while(cur && cur->next && cur->next->value.b < row.b)
        {
            // Move to next when next value < value of input row
            cur = cur->next;
        }

        node->next = cur->next;
        cur->next = node;

        // Switch values between root node vs ->next
        if (cur->value.b > cur->next->value.b)
        {
            Row tmp = cur->next->value;
            cur->next->value = cur->value;
            cur->value = tmp;
        }

        if(!(rmap+row.b))
        {
            // init the map entry if necessary.
            rmap[row.b].point = node;
        }
        
        // printf("DEBUG: node(%d,%d) is inserted after node(%d,%d)\n", row.a,row.b, cur->value.a, cur->value.b);
    }
}

/**
 * @brief Handle Row according to task1.
 * 
 * @param row immutable object for all row.
 * @return 0 row is accepted
 * @return 1 move to left
 * @return 2 move to right
 */
uint8_t task4_handle(Row row)
{
    // a in (1000, 2000, 3000) and b between 10 and 50
    if (row.a >= 1000 && row.a < 99000 && row.b >= 10 && row.b < 50)
    {
        ordered_insert(row);
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
void task3(const Row *rows, int nrows)
{
    scan_process(rows, nrows, task4_handle);
}

int main(void)
{
    rmap = calloc(100, sizeof(MapEntry));

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

    // Execute task1
    task3(rows, 6);

    /*
    Row range_left  = {2000,10};
    Row range_right = {2000,50};

    printf("(%d,%d)[%d]->(%d,%d)[%d]\n", 
            range_left.a, range_left.b, search_right_most_rowidx(rows, 6, 0, 5, range_left),
            range_right.a, range_right.b, search_left_most_rowidx(rows, 6, 0, 5, range_right));
    */
    // Destroy generated dataset.
    // free(rows);
}
 