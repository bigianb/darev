#pragma once

struct Matrix_3x3
{
    float cell[3][3];
};

// 4 columns, 3 rows
struct Matrix_3x4
{
    // cell[col][row]
    // array of 4 columns, each of which is 3 floats
    float cell[4][3];
};

struct Matrix_4x4
{
    // cell[col][row]
    float cell[4][4];
};


inline
void makeIdentityMtx_3x3(Matrix_3x3 *mtx)
{
    mtx->cell[0][0] = 1.0f;
    mtx->cell[0][1] = 0.0f;
    mtx->cell[0][2] = 0.0f;
    mtx->cell[1][0] = 0.0f;
    mtx->cell[1][1] = 1.0f;
    mtx->cell[1][2] = 0.0f;
    mtx->cell[2][0] = 0.0f;
    mtx->cell[2][1] = 0.0f;
    mtx->cell[2][2] = 1.0f;
}

inline
void makeIdentityMtx_3x4(Matrix_3x4 *mtx)
{
    mtx->cell[0][0] = 1.0f;
    mtx->cell[0][1] = 0.0f;
    mtx->cell[0][2] = 0.0f;
    mtx->cell[1][0] = 0.0f;
    mtx->cell[1][1] = 1.0f;
    mtx->cell[1][2] = 0.0f;
    mtx->cell[2][0] = 0.0f;
    mtx->cell[2][1] = 0.0f;
    mtx->cell[2][2] = 1.0f;
    mtx->cell[3][0] = 0.0f;
    mtx->cell[3][1] = 0.0f;
    mtx->cell[3][2] = 0.0f;
}

Matrix_3x4 * translateMtx3x4(float x, float y, float z, Matrix_3x4 *mtx)
{
    mtx->cell[3][0] = mtx->cell[3][0] + mtx->cell[0][0] * x + mtx->cell[1][0] * y + mtx->cell[2][0] * z;
    mtx->cell[3][1] = mtx->cell[3][1] + mtx->cell[0][1] * x + mtx->cell[1][1] * y + mtx->cell[2][1] * z;
    mtx->cell[3][2] = mtx->cell[3][2] + mtx->cell[0][2] * x + mtx->cell[1][2] * y + mtx->cell[2][2] * z;

    return mtx;
}

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
    
    Matrix_4x4 tmp_4x4Mtx;

    float (*pm2_col1)[4] = &m2_4x4->cell[1];
    float (*pm2_col2)[4] = &m2_4x4->cell[2];
    float (*pm2_col3)[4] = &m2_4x4->cell[3];
    int col = 0;
    // First do the 3x3 sub-matrix of the output
    do {
        float* outputCell = tmp_4x4Mtx.cell[col];
        
        // vertical slice
        float m1_row_0 = m1_3x4->cell[col][0];
        float m1_row_1 = m1_3x4->cell[col][1];
        float m1_row_2 = m1_3x4->cell[col][2];

        // horiz slice
        float *pfVar7 = (float *)m2_4x4;
        float (*pafVar8)[4] = pm2_col1;
        float (*pafVar9)[4] = pm2_col2;
        
        int iVar11 = 3;
        do {
            float fVar14 = *pfVar7;
            
            float* pfVar4 = *pafVar8;
            pfVar7 = pfVar7 + 1;
            float* pfVar5 = *pafVar9;
            pafVar9 = (float (*) [4])(*pafVar9 + 1);
            pafVar8 = (float (*) [4])(*pafVar8 + 1);

            *outputCell++ = m1_row_0 * fVar14 + m1_row_1 * *pfVar4 + m1_row_2 * *pfVar5;

            iVar11 -= 1;
        } while (iVar11 >= 0);
        col++;
    } while (col < 3);

    // populate the 4th column
    float *pafVar9 = tmp_4x4Mtx.cell[3];
    int iVar12 = 3;
    float fVar17 = m1_3x4->cell[3][0];
    float fVar15 = m1_3x4->cell[3][1];
    float fVar16 = m1_3x4->cell[3][2];
    do {
        float fVar14 = *(float *)m2_4x4;
        iVar12 += -1;
        float *pfVar7 = *pm2_col1;
        m2_4x4 = (Matrix_4x4 *)((int)m2_4x4 + 4);
        float *pfVar4 = *pm2_col2;
        float *pfVar5 = *pm2_col3;
        pm2_col3 = (float (*) [4])(*pm2_col3 + 1);
        pm2_col2 = (float (*) [4])(*pm2_col2 + 1);
        pm2_col1 = (float (*) [4])(*pm2_col1 + 1);
        *pafVar9 = fVar17 * fVar14 + fVar15 * *pfVar7 + fVar16 * *pfVar4 + *pfVar5;
        pafVar9 += 4;
    } while (-1 < iVar12);

  return mOut_4x4;
}

