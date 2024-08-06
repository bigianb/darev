#include "matrix.h"

Matrix_4x4 * matrixMul3x4_4x4(Matrix_4x4 *mOut_4x4, Matrix_3x4 *m1_3x4, Matrix_4x4 *m2_4x4)
{
    // i = row, j = col
    // Cij = Sum Over k of Aik Bkj
 
    // Pre multiply so mOut = m2 . m1

    /*
    
     m2           m1

    a b c d      A B C D     aA + bE + cI
    e f g h      E F G H     eA + fE + gI
    i j k l      I J K L
    m n o p      0 0 0 1

    */

    int col = 0;
    // First do the 3 columns of the output
    do {        
        // vertical slice
        float m1_row_0 = m1_3x4->cell[col][0];
        float m1_row_1 = m1_3x4->cell[col][1];
        float m1_row_2 = m1_3x4->cell[col][2];
        // row 3 of m1 is 0
        
        int row = 0;
        do {
            mOut_4x4->cell[col][row] = m1_row_0 * m2_4x4->cell[0][row] + m1_row_1 * m2_4x4->cell[1][row] + m1_row_2 * m2_4x4->cell[2][row];
            ++row;
        } while (row < 4);
        ++col;
    } while (col < 3);

    // populate the 4th column    
    float D = m1_3x4->cell[3][0];
    float H = m1_3x4->cell[3][1];
    float L = m1_3x4->cell[3][2];
    int outRow = 0;
    do {
        mOut_4x4->cell[3][outRow] = D * m2_4x4->cell[0][outRow] + H * m2_4x4->cell[1][outRow] + L * m2_4x4->cell[2][outRow] + m2_4x4->cell[3][outRow];
        ++outRow;
    } while (outRow < 4);

  return mOut_4x4;
}
