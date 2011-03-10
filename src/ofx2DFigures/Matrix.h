#ifndef MATRIX_H
#define MATRIX_H

#include "ofMain.h"

class Matrix
{
    public:
        GLdouble data[16];

        Matrix()
        {
            SetIdentity();
        }

        Matrix(Matrix const & m)
        {
            for(int i = 0; i < 16; i++)
            {
                data[i] = m.data[i];
            }
        }

        void SetIdentity()
        {
            for(int i = 0; i < 16; i++)
            {
                data[i] = 0;
            }
            data[0] = 1;
            data[5] = 1;
            data[10] = 1;
            data[15] = 1;
        }

        ofPoint Transform(const ofPoint& point)
        {
            ofPoint point_r = ofPoint();
            point_r.x = data[0] * point.x + data[4] * point.y + data[8] * point.z + data[12];
            point_r.y = data[1] * point.x + data[5] * point.y + data[9] * point.z + data[13];
            point_r.z = data[2] * point.x + data[6] * point.y + data[10] * point.z + data[14];
            return point_r;
        }

        Matrix& operator=(Matrix const &m)
        {
            for(int i = 0; i < 16; i++)
            {
                data[i] = m.data[i];
            }
            return *this;
        }

        Matrix operator* (const Matrix& matrix) const
        {
            Matrix matrixR = Matrix();
            for (int i = 0 ; i < 4 ; i++ )
            {
                for (int j = 0 ; j < 4 ; j++ )
                {
                    matrixR.data[i+(j*4)] = 0;
                    for (int k = 0 ; k < 4 ; k++ )
                    {
                        matrixR.data[i+(j*4)]+= data[i+(k*4)]*matrix.data[k+(j*4)];
                    }
                }
            }
            return matrixR;
        }

        Matrix GetInverse()
        {
            int n = 4;
            Matrix matrix_r = Matrix();
            double a[16];
            double b[16];
            for ( int i = 0; i < n*n; i ++)
            {
                a[i] = data[i];
                b[i] = 0;
                matrix_r.data[i]=0;
            }
            for ( int i = 0; i < n; i ++) b[(i*n)+i] = 1.0;

            for (int k = 0; k < n-1; k ++)
            {
                for (int i = k+1; i < n; i ++)
                {
                    for (int s = 0; s < n; s++)
                    {
                        b[(i*n)+s] -= a[(i*n)+k] * b[(k*n)+s] / a[(k*n)+k];
                    }
                    for (int j = k+1; j < n; j++)
                    {
                        a[(i*n)+j] -= a[(i*n)+k] * a[(k*n)+j] / a[(k*n)+k];
                    }
                }
            }
            for (int s = 0; s < n; s++)
            {
                matrix_r.data[((n-1)*n)+s] = b[((n-1)*n)+s] / a[((n-1)*n)+(n-1)];
                for (int i = n-2; i >= 0; i--)
                {
                    matrix_r.data[(i*n)+s] = b[(i*n)+s] / a[(i*n)+i];
                    for (int k = n-1; k > i; k--)
                    {
                        matrix_r.data[(i*n)+s] -= a[(i*n)+k] * matrix_r.data[(k*n)+s] / a[(i*n)+i];
                    }
                }
            }
            return matrix_r;
        }
};

#endif // MATRIX_H
