/*
 *  
 *###########################################################################
 * Copyright (c) 2010, Los Alamos National Security, LLC.
 * All rights reserved.
 * 
 *  Copyright 2010. Los Alamos National Security, LLC. This software was
 *  produced under U.S. Government contract DE-AC52-06NA25396 for Los
 *  Alamos National Laboratory (LANL), which is operated by Los Alamos
 *  National Security, LLC for the U.S. Department of Energy. The
 *  U.S. Government has rights to use, reproduce, and distribute this
 *  software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
 *  LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
 *  FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
 *  derivative works, such modified software should be clearly marked,
 *  so as not to confuse it with the version available from LANL.
 *
 *  Additionally, redistribution and use in source and binary forms,
 *  with or without modification, are permitted provided that the
 *  following conditions are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 * 
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided 
 *      with the distribution.
 *
 *    * Neither the name of Los Alamos National Security, LLC, Los
 *      Alamos National Laboratory, LANL, the U.S. Government, nor the
 *      names of its contributors may be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
 *  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL LOS ALAMOS NATIONAL SECURITY, LLC OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 *  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *  SUCH DAMAGE.
 */ 

#ifndef SCOUT_MATRIX_H_
#define SCOUT_MATRIX_H_

#include "scout/Runtime/opengl/vectors.h"

namespace scout {

  template<class T>
  class matrix4
  {

   public:

    matrix4()
    { setIdentity(); }

    matrix4(T t) 
    { setValue(t); }

    matrix4(const T *mp)
    { setValue(mp); }

    matrix4(T v00, T v01, T v02, T v03,
            T v10, T v11, T v12, T v13,
            T v20, T v21, T v22, T v23,
            T v30, T v31, T v32, T v33)
    {
      _11 = v00; _12 = v01; _13 = v02; _14 = v03;
      _21 = v10; _22 = v11; _23 = v12; _24 = v13;
      _31 = v20; _32 = v21; _33 = v22; _34 = v23;
      _41 = v30; _42 = v31; _43 = v32; _44 = v33;                  
    }

    void getValue(T *mp) const
    {
      int c = 0;
      for(int j=0; j < 4; j++)
        for(int i=0; i < 4; i++)
          mp[c++] = element(i,j);
    }

    T * getValue()
    { return _values; }
    
    const T * getValue() const
    { return _values; }

    void setValue(T * mp)
    {
      int c = 0;
      for(int j = 0; j < 4; j++)
        for(int i = 0; i < 4; i++)
          element(i,j) = mp[c++];
    }

    void setValue(T r)
    {
      for(int i=0; i < 4; i++)
        for(int j=0; j < 4; j++)
          element(i,j) = r;
    }

    void setIdentity()
    {
      element(0,0) = 1.0; element(0,1) = 0.0; element(0,2) = 0.0; element(0,3) = 0.0;
      element(1,0) = 0.0; element(1,1) = 1.0; element(1,2) = 0.0; element(1,3) = 0.0;
      element(2,0) = 0.0; element(2,1) = 0.0; element(2,2) = 1.0; element(2,3) = 0.0;
      element(3,0) = 0.0; element(3,1) = 0.0; element(3,2) = 0.0; element(3,3) = 1.0;
    }

    // set a uniform scale
    void setScale(T s)
    {
      element(0,0) = s;
      element(1,1) = s;
      element(2,2) = s;
    }

    void setScale(const glvec3<T> & s)
    {
      element(0,0) = s[0];
      element(1,1) = s[1];
      element(2,2) = s[2];
    }


    void setTranslate(const glvec3<T> & t)
    {
      element(0,3) = t[0];
      element(1,3) = t[1];
      element(2,3) = t[2];      
    }

    void setRow(int r, const glvec4<T> & t)
    {
      for (int i = 0; i < 4; i++)
        element(r,i) = t[i];
    }

    void setColumn(int c, const glvec4<T> & t)
    {
      for (int i = 0; i < 4; i++)
        element(i,c) = t[i];
    }

    glvec4<T> getRow(int r) const {
      glvec4<T> v;
      for (int i = 0; i < 4; i++) v[i] = element(r,i);
      return v;
    }

    glvec4<T> getColumn(int c) const {
      glvec4<T> v;
      for (int i = 0; i < 4; i++) v[i] = element(i,c);
      return v;
    }

    friend matrix4 inverse(const matrix4 & m) {
      matrix4 minv;

      T r1[8], r2[8], r3[8], r4[8];
      T *s[4], *tmprow;

      s[0] = &r1[0];
      s[1] = &r2[0];
      s[2] = &r3[0];
      s[3] = &r4[0];

      register int i,j,p,jj;
      for(i=0;i<4;i++) {
        for(j=0;j<4;j++) {
          s[i][j] = m.element(i,j);
          if(i==j) s[i][j+4] = 1.0;
          else     s[i][j+4] = 0.0;
        }
      }
      T scp[4];
      for(i=0;i<4;i++) {
        scp[i] = T(fabs(s[i][0]));
        for(j=1;j<4;j++)
          if(T(fabs(s[i][j])) > scp[i]) scp[i] = T(fabs(s[i][j]));
        if(scp[i] == 0.0) return minv; // singular matrix!
      }

      int pivot_to;
      T scp_max;
      for(i=0;i<4;i++) {
        // select pivot row
        pivot_to = i;
        scp_max = T(fabs(s[i][i]/scp[i]));
        // find out which row should be on top
        for(p=i+1;p<4;p++)
          if (T(fabs(s[p][i]/scp[p])) > scp_max) {
            scp_max = T(fabs(s[p][i]/scp[p]));
            pivot_to = p;
          }
        // Pivot if necessary
        if(pivot_to != i) {
          tmprow = s[i];
          s[i] = s[pivot_to];
          s[pivot_to] = tmprow;
          T tmpscp;
          tmpscp = scp[i];
          scp[i] = scp[pivot_to];
          scp[pivot_to] = tmpscp;
        }

        T mji;
        // perform gaussian elimination
        for(j=i+1;j<4;j++) {
          mji = s[j][i]/s[i][i];
          s[j][i] = 0.0;
          for(jj=i+1;jj<8;jj++)
            s[j][jj] -= mji*s[i][jj];
        }
      }
      if(s[3][3] == 0.0) return minv; // singular matrix!

      //
      // Now we have an upper triangular matrix.
      //
      //  x x x x | y y y y
      //  0 x x x | y y y y 
      //  0 0 x x | y y y y
      //  0 0 0 x | y y y y
      //
      //  we'll back substitute to get the inverse
      //
      //  1 0 0 0 | z z z z
      //  0 1 0 0 | z z z z
      //  0 0 1 0 | z z z z
      //  0 0 0 1 | z z z z 
      //

      T mij;
      for(i=3;i>0;i--) {
        for(j=i-1;j > -1; j--) {
          mij = s[j][i]/s[i][i];
          for(jj=j+1;jj<8;jj++)
            s[j][jj] -= mij*s[i][jj];
        }
      }

      for(i=0;i<4;i++)
        for(j=0;j<4;j++)
          minv(i,j) = s[i][j+4] / s[i][i];

      return minv;
    }

    friend matrix4 transpose(const matrix4 & m) {
      matrix4 mtrans;

      for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
          mtrans(i,j) = m.element(j,i);   
      return mtrans;
    }

    matrix4 & operator *= (const matrix4 & rhs) {
      matrix4 mt(*this);
      set_value(T(0));

      for(int i=0; i < 4; i++)
        for(int j=0; j < 4; j++)
          for(int c=0; c < 4; c++)
            element(i,j) += mt(i,c) * rhs(c,j);
      return *this;
    }

    friend matrix4 operator * (const matrix4 & lhs, const matrix4 & rhs) {
      matrix4 r(T(0));

      for(int i=0; i < 4; i++)
        for(int j=0; j < 4; j++)
          for(int c=0; c < 4; c++)
            r.element(i,j) += lhs(i,c) * rhs(c,j);
      return r;
    }

    // dst = M * src
    glvec4<T> operator *(const glvec4<T> &src) const {
      glvec4<T> r;
      for (int i = 0; i < 4; i++)
        r[i]  = (src[0] * element(i,0) + src[1] * element(i,1) +
                 src[2] * element(i,2) + src[3] * element(i,3));
      return r;
    }

    // dst = src * M
    friend glvec4<T> operator *(const glvec4<T> &lhs, const matrix4 &rhs) {
      glvec4<T> r;
      for (int i = 0; i < 4; i++)
        r[i]  = (lhs[0] * rhs.element(0,i) + lhs[1] * rhs.element(1,i) +
                 lhs[2] * rhs.element(2,i) + lhs[3] * rhs.element(3,i));
      return r;
    }

    T & operator () (int row, int col) {
      return element(row,col);
    }

    const T & operator () (int row, int col) const {
      return element(row,col);
    }

    T & element (int row, int col) {
      return _values[row | (col<<2)];
    }

    const T & element (int row, int col) const {
      return _values[row | (col<<2)];
    }

    matrix4 & operator *= (const T & r) {
      for (int i = 0; i < 4; ++i) {
        element(0,i) *= r;
        element(1,i) *= r;
        element(2,i) *= r;
        element(3,i) *= r;
      }
      return *this;
    }

    matrix4 & operator += (const matrix4 & mat) {
      for (int i = 0; i < 4; ++i) {
        element(0,i) += mat.element(0,i);
        element(1,i) += mat.element(1,i);
        element(2,i) += mat.element(2,i);
        element(3,i) += mat.element(3,i);
      }
      return *this;
    }

    
    friend bool operator == (const matrix4 & lhs, const matrix4 & rhs) {
      bool r = true;
      for (int i = 0; i < 16; i++)
        r &= lhs._values[i] == rhs._values[i];
      return r;
    }

    friend bool operator != (const matrix4 & lhs, const matrix4 & rhs)  {
      bool r = true;
      for (int i = 0; i < 16; i++)
        r &= lhs._values[i] != rhs._values[i];
      return r;
    }

    union {
      struct {
        T _11, _12, _13, _14;   // standard names for components
        T _21, _22, _23, _24;   // standard names for components
        T _31, _32, _33, _34;   // standard names for components
        T _41, _42, _43, _44;   // standard names for components
      };
      T _values[16];     // array access
    };
  };

};

#endif
