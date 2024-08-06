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

inline
Matrix_3x4 * translateMtx3x4(float x, float y, float z, Matrix_3x4 *mtx)
{
    mtx->cell[3][0] = mtx->cell[3][0] + mtx->cell[0][0] * x + mtx->cell[1][0] * y + mtx->cell[2][0] * z;
    mtx->cell[3][1] = mtx->cell[3][1] + mtx->cell[0][1] * x + mtx->cell[1][1] * y + mtx->cell[2][1] * z;
    mtx->cell[3][2] = mtx->cell[3][2] + mtx->cell[0][2] * x + mtx->cell[1][2] * y + mtx->cell[2][2] * z;

    return mtx;
}

Matrix_4x4 * matrixMul3x4_4x4(Matrix_4x4 *mOut_4x4, Matrix_3x4 *m1_3x4, Matrix_4x4 *m2_4x4);

