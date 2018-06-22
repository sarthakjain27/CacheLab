/* Name:Sarthak Jain
 * Andrew ID: sarthak3
 *
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);
 * A is the source matrix, B is the destination
 * tmp points to a region of memory able to hold TMPCOUNT (set to 256) doubles as temporaries
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 2KB direct mapped cache with a block size of 64 bytes.
 *
 * Programming restrictions:
 *   No out-of-bounds references are allowed
 *   No alterations may be made to the source array A
 *   Data in tmp can be read or written
 *   This file cannot contain any local or global doubles or arrays of doubles
 *   You may not use unions, casting, global variables, or 
 *     other tricks to hide array data in other forms of local or global memory.
 */ 
#include <stdio.h>
#include <stdbool.h>
#include "cachelab.h"
#include "contracts.h"

/* Forward declarations */
bool is_transpose(size_t M, size_t N, double A[N][M], double B[M][N]);
void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);
void trans_tmp(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";

/*Using same block strategy for our two matrix size(32*32, 63*65)
 *We are just changing the block size depending on what matrix is passed.
 *
 *The function makes use of 4 loops. The outer two loops is to divide array A into smaller block_size*block_size matrices.
 *We then access the smaller array A block in column major order.
 *
 *The inner two loops is to access elements of the created block. We do that in row-major order.
 *
 *Handling of diagonal elements is separate to avoic conflict miss, since then A & B are in same SET but with differernt TAG. Since it is Direct Mapped Cachec. Conflict Mises will occur. So to avoid that we store diagonal elements in temp array and access that element outside the loop.
 *
 *Clock cycles for various block sizes for each test case sized matrices was checked and then the optimal was used.
 */
void transpose_submit(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp)
{
    /*
     * This is a good place to call your favorite transposition functions
     * It's OK to choose different functions based on array size, but
     * your code must be correct for all values of M and N
     */
	int block_size=0;//dividing Array A into smaller block_size*block_size arrays and doing their trasnpose
	int rowIter_block=0,colIter_block=0;//defining the starting row and column index of each block. The blocks in A are shifting column major wise
	int rowIndex=0,colIndex=0;//variable to iterate over each element in the block
	int diag_pos=0;//to store index of diagonal elements
	if(M==32)//separately handling case of square and non-square test case
		block_size=8;//tried block size from 2,4,8,16,32. Clock cycle for 2:73632, 4:49248, 8:37536, 16:119520
	else//for 63*65
		block_size=4;//tried block size form 2,4,8,16,32. Clock cycle for 2:331256, 4:261272, 8:396056, 16:442904, 32:468440
	for(colIter_block=0;colIter_block<M;colIter_block+=block_size)
	{
		for(rowIter_block=0;rowIter_block<N;rowIter_block+=block_size)
		{
			for(rowIndex=rowIter_block;rowIndex<N && rowIndex<rowIter_block+block_size;rowIndex++)//for non-square rowIndex might exceed N so a extra check for that
			{
				for(colIndex=colIter_block;colIndex<M && colIndex<colIter_block+block_size;colIndex++)//for non-square colIndex might exceed M so an extr check for that
				{
					if(colIndex!=rowIndex)
					{
						B[colIndex][rowIndex]=A[rowIndex][colIndex];//accessing elements of A row-major wise
					}
					else
					{
						tmp[255]=A[rowIndex][rowIndex];//for diagonal elements, A & B access same SET of cahce but has different TAG. So to avoid conflict misses
						diag_pos=rowIndex;//...we handle the diagonal element by storing them in temp array and accessing it outside the loop
					}
				}
				if(colIter_block==rowIter_block)
					B[diag_pos][diag_pos]=tmp[255];
			}
		}
	}
	ENSURES(istranspose(M,N,A,B));
}
/* 
 * You can define additional transpose functions below. We've defined
 * some simple ones below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";

/*
 * The following shows an example of a correct, but cache-inefficient
 *   transpose function.  Note the use of macros (defined in
 *   contracts.h) that add checking code when the file is compiled in
 *   debugging mode.  See the Makefile for instructions on how to do
 *   this.
 *
 *   IMPORTANT: Enabling debugging will significantly reduce your
 *   cache performance.  Be sure to disable this checking when you
 *   want to measure performance.
 */
void trans(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp)
{
    size_t i, j;

    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            B[j][i] = A[i][j];
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * This is a contrived example to illustrate the use of the temporary array
 */

char trans_tmp_desc[] =
    "Simple row-wise scan transpose, using a 2X2 temporary array";

void trans_tmp(size_t M, size_t N, double A[N][M], double B[M][N], double *tmp)
{
    size_t i, j;
    /* Use the first four elements of tmp as a 2x2 array with row-major ordering. */
    REQUIRES(M > 0);
    REQUIRES(N > 0);

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int di = i%2;
            int dj = j%2;
            tmp[2*di+dj] = A[i][j];
            B[j][i] = tmp[2*di+dj];
        }
    }    

    ENSURES(is_transpose(M, N, A, B));
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_tmp, trans_tmp_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
bool is_transpose(size_t M, size_t N, double A[N][M], double B[M][N])
{
    size_t i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return false;
            }
        }
    }
    return true;
}

