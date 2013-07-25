
#include <math.h>
#include <string.h>
#include <iostream>
#include <assert.h>


#ifndef PMATH_H
#define PMATH_H

typedef float real_t;

#define PI     3.14159      /// pi
#define TWO_PI 6.18308      /// 2*pi
#define PI_180 0.0174532925 /// Pi / 180
#define EPSILON 0.001 //Epsilon

#define VEC_4TO3(v) Vector3((v).x, (v).y, (v).z)
#define VEC_3TO4(v) Vector4((v).x, (v).y, (v).z, 0)
#define CONV_COLOR3(v) Color3((v).r, (v).g, (v).b)
#define MAX(x, y) (x) > (y) ? (x) : (y)
#define MIN(x, y) (x) < (y) ? (x) : (y)

#define EQ(x, y) (((x) <= ((y) + EPSILON)) && ((x) >= ((y) - EPSILON)))


class Color3 {
public:
    Color3(real_t R, real_t G, real_t B) { data[0] = R; data[1] = G; data[2] = B; };

#define X Color3
#define N 3
#define T real_t
#define VECOP_PCW(op) { for (unsigned i = 0; i < N; ++i) data[i] op rhs.data[i]; return *this; }
#define VECOP_SCA(op) { for (unsigned i = 0; i < N; ++i) data[i] op rhs;         return *this; }


    X() : r(0),g(0),b(0) {};
    X(const X &copy) { memcpy(data, copy.data, sizeof(T)*N); };

    T  operator[](int i) const { return data[i]; }
    T& operator[](int i)       { return data[i]; }

    inline real_t max() const{ return MAX(MAX(r, g), b); };

    inline X& operator  = (const X& rhs) VECOP_PCW( =); /// equality assignment
    inline X& operator += (const X& rhs) VECOP_PCW(+=); /// piecewise addition operator
    inline X& operator -= (const X& rhs) VECOP_PCW(-=); /// piecewise subtraction operator
    inline X& operator *= (const X& rhs) VECOP_PCW(*=); /// piecewise subtraction operator

    inline X  operator  + (const X& rhs) const { return X(*this) += rhs; } /// piecewise addition
    inline X  operator  - (const X& rhs) const { return X(*this) -= rhs; } /// piecewise subtraction
    inline X  operator  * (const X& rhs) const { return X(*this) *= rhs; } /// piecewise multiplication

    inline X& operator += (const T  rhs)  VECOP_SCA(+=); /// scalar addition operator
    inline X& operator -= (const T  rhs)  VECOP_SCA(-=); /// scalar subtraction operator
    inline X& operator *= (const T  rhs)  VECOP_SCA(*=); /// scalar multiplication operator
    inline X& operator /= (const T  rhs)  VECOP_SCA(/=); /// scalar division operator

    inline X  operator  + (const T  rhs) const { return X(*this) += rhs; } /// piecewise addition
    inline X  operator  - (const T  rhs) const { return X(*this) -= rhs; } /// piecewise subtraction
    inline X  operator  * (const T  rhs) const { return X(*this) *= rhs; } /// piecewise multiplication
    inline X  operator  / (const T  rhs) const { return X(*this) /= rhs; } /// piecewise multiplication

    /// Converts this RGB [0,1] color to the corresponding ARGB 32-bit integer
    inline int toInt32() {
        real_t R = MIN(r, 1), G = MIN(g, 1), B = MIN(b, 1);
        return (255 << 24) | ((int)(R*255) << 16) | ((int)(G*255) << 8) | (int)(B*255);
    }

    union {
        T data[N]; /// data that holds the RGB color information
        struct { T r,g,b; };
    };

#undef X
#undef N
#undef T
#undef VECOP_PCW
#undef VECOP_SCA

};
static const Color3 COLOR3_BLACK = Color3(0,0,0);
static const Color3 COLOR3_WHITE = Color3(1,1,1);


struct Vector3  {

    Vector3(real_t v0 = 0, real_t v1 = 0, real_t v2 = 0) : x(v0), y(v1), z(v2) { }
    Vector3(real_t *data) { x = data[0]; y = data[1]; z = data[2]; };

    static inline Vector3 zero() { return Vector3(0,0,0); };

    /**
      Randomly samples a cosine weighted vector in the hemisphere
      **/
    static Vector3 cosRandom() {
        return Vector3();
    }

#define VECOP_PCW(op) { x op rhs.x; y op rhs.y; z op rhs.z; return *this; }
#define VECOP_SCA(op) { x op rhs;   y op rhs  ; z op rhs  ; return *this; }
#define X Vector3

    inline X& operator  = (const X& rhs) VECOP_PCW( =); /// equality assignment
    inline X& operator += (const X& rhs) VECOP_PCW(+=); /// piecewise addition operator
    inline X& operator -= (const X& rhs) VECOP_PCW(-=); /// piecewise subtraction operator


    inline X  operator  + (const X& rhs) const { return X(*this) += rhs; } /// piecewise addition
    inline X  operator  - (const X& rhs) const { return X(*this) -= rhs; } /// piecewise subtraction

    inline X& operator += (const real_t  rhs)  VECOP_SCA(+=); /// scalar addition operator
    inline X& operator -= (const real_t  rhs)  VECOP_SCA(-=); /// scalar subtraction operator
    inline X& operator *= (const real_t  rhs)  VECOP_SCA(*=); /// scalar multiplication operator
    inline X& operator /= (const real_t  rhs)  VECOP_SCA(/=); /// scalar division operator

    inline X  operator  + (const real_t  rhs) const { return X(*this) += rhs; } /// piecewise addition
    inline X  operator  - (const real_t  rhs) const { return X(*this) -= rhs; } /// piecewise subtraction
    inline X  operator  * (const real_t  rhs) const { return X(*this) *= rhs; } /// piecewise multiplication
    inline X  operator  / (const real_t  rhs) const { return X(*this) /= rhs; } /// piecewise multiplication

#undef X
#undef VECOP_PCW
#undef VECOP_SCA

    inline real_t dot(const Vector3 &rhs) const {
            return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    inline real_t normalize() {
            real_t m = getMagnitude();
            x /= m;
            y /= m;
            z /= m;
            return m;
    }

    inline Vector3 getNormalized() {
            real_t m = getMagnitude();
            return Vector3(x / m, y / m, z / m);
    }

    inline real_t getMagnitude() const {
            return sqrt(getMagnitude2());
    }

    inline real_t getMagnitude2() const {
            return x * x + y * y + z * z;
    }

    inline real_t getDistance(const Vector3 &rhs) const {
            return sqrt(getDistance2(rhs));
    }

    inline real_t getDistance2(const Vector3 &rhs) const {
            return (rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y) +
                    (rhs.z - z) * (rhs.z - z);
    }

    inline Vector3 cross(const Vector3& rhs) const {
            return Vector3(data[1] * rhs.data[2] - data[2] * rhs.data[1],
                           data[2] * rhs.data[0] - data[0] * rhs.data[2],
                           data[0] * rhs.data[1] - data[1] * rhs.data[0]);
    }

    inline bool operator==(const Vector3 &rhs) {
            return (x == rhs.x && y == rhs.y && z == rhs.z);
    }

    inline bool operator!=(const Vector3 &rhs) {
            return (x != rhs.x || y != rhs.y || z != rhs.z);
    }
    /**
       Assuming *this is incident to the surface and the result is pointing
       away from the surface.
    **/
    inline Vector3 reflectVector(const Vector3 &normal) {
          return (*this) - (normal * ((*this).dot(normal))) * 2.0;
    }

    inline Vector3 refractVector(const Vector3 &normal, real_t in,
                                          real_t out) {
        real_t n = (sin(out) / sin(in)); //Ratio of indexes of refraction (Snells law)

        //N - normal U - incident vector, n - ratio of refraction indexes
        //The full equation would be nI-N(n(N DOT U) + sqrt(1-n^2(1-(N DOT U)^2)))
        //However, this can be simplified at the cost of some accuracy to a
        //simpler equation
        //U-(1-n)N(N DOT U)
        //Taken from Advanced graphics programming using openGL Vol 385.
        //Tom McReynolds, David Blythe

        //Assuming *this is incident to the surface, and the result is
        //pointing away from the surface
        return (*this) - normal*(normal.dot(*this)) * (1 - n);
    }

    union {
        real_t data[3];
        struct {
            real_t x, y, z;
        };
    };
};

template<typename T>
struct vec4 {
#define vec4_T vec4<T>

    inline vec4_T() : x(0), y(0), z(0), w(0) {}
    inline vec4_T(T v0, T v1, T v2, T v3) : x(v0), y(v1), z(v2), w(v3) {}
    inline vec4_T(const T *data) { x = data[0]; y = data[1]; z = data[2]; w = data[3]; };

    static inline vec4_T zero() { return vec4_T(0,0,0,0); };

    inline vec4_T& homogenize() {
        w = 1;
        return *this;
    }

    inline vec4_T& unhomgenize() {
        w = 0;
        return *this;
    }

    static inline vec4_T cosRandom(vec4_T &normal, T u, T v) {
        const T theta = acos(sqrt(u));
        const T phi = 2.0 * PI * v;
        Vector3 w(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
        w.normalize();

        //Convert hemisphere
        vec4_T U, V;
        normal.getBasis(U , V);
        return U * w.x + normal * w.y + V * w.z;
    }

    inline void getBasis(vec4_T &u, vec4_T &v) {
        normalize();
        const vec4_T &n = *this;
        vec4_T up = vec4_T(0, 1, 0, 0);
        if(fabs(up.dot(n)) > 0.8) up = vec4_T(1, 0, 0, 0);
        u = up.cross(n).getNormalized();
        v = n.cross(u).getNormalized();
    }

    inline void reflect(const vec4_T &normal) {
          //return (*this) - (normal * ((*this).dot(normal))) * 2.0;
        (*this) -= normal * ((*this).dot(normal)) * 2.0;
        //assert(this->data[3] == 0);
    }

    inline T  operator[](unsigned i) const { return data[i]; }
    inline T& operator[](unsigned i)       { return data[i]; }

#define VECOP_PCW(op) { x op rhs.x; y op rhs.y; z op rhs.z; w op rhs.w; return *this; }
#define VECOP_SCA(op) { x op rhs  ; y op rhs  ; z op rhs  ; w op rhs  ; return *this; }
#define X vec4_T

    inline X& operator  = (const X& rhs) VECOP_PCW( =); /// equality assignment
    inline X& operator += (const X& rhs) VECOP_PCW(+=); /// piecewise addition operator
    inline X& operator -= (const X& rhs) VECOP_PCW(-=); /// piecewise subtraction operator


    inline X  operator  + (const X& rhs) const { return X(*this) += rhs; } /// piecewise addition
    inline X  operator  - (const X& rhs) const { return X(*this) -= rhs; } /// piecewise subtraction

    inline X& operator += (const T  rhs)  VECOP_SCA(+=); /// scalar addition operator
    inline X& operator -= (const T  rhs)  VECOP_SCA(-=); /// scalar subtraction operator
    inline X& operator *= (const T  rhs)  VECOP_SCA(*=); /// scalar multiplication operator
    inline X& operator /= (const T  rhs)  VECOP_SCA(/=); /// scalar division operator

    inline X  operator  + (const T  rhs) const { return X(*this) += rhs; } /// piecewise addition
    inline X  operator  - (const T  rhs) const { return X(*this) -= rhs; } /// piecewise subtraction
    inline X  operator  * (const T  rhs) const { return X(*this) *= rhs; } /// piecewise multiplication
    inline X  operator  / (const T  rhs) const { return X(*this) /= rhs; } /// piecewise multiplication

#undef X
#undef VECOP_PCW
#undef VECOP_SCA

    inline T max() const {
        return MAX(MAX(MAX(x, y), z), w);
    }

    inline T dot(const vec4_T &rhs) const {
            return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    inline T normalize() {
            T m = 1.0 / sqrt(x*x + y*y + z*z + w*w);
#pragma vector align
            for (unsigned i = 0; i < 4; ++i)
                data[i] *= m;
            return 1.0/m;
    }

    inline vec4_T getNormalized() {
            T m = 1.0 / sqrt(x*x + y*y + z*z + w*w);
            return vec4_T(x*m, y*m, z*m, w*m);
    }

    inline T getMagnitude() const {
            return sqrt(getMagnitude2());
    }

    inline T getMagnitude2() const {
            return x * x + y * y + z * z + w * w;
    }

    inline T getDistance(const vec4_T &rhs) const {
            return sqrt(getDistance2(rhs));
    }

    inline T getDistance2(const vec4_T &rhs) const {
            return (rhs.x - x) * (rhs.x - x) + (rhs.y - y) * (rhs.y - y) +
                    (rhs.z - z) * (rhs.z - z) + (rhs.w - w) * (rhs.w - w);
    }

    inline vec4_T getHomogenized() {
        return vec4_T(*this).homogenize();
    }

    inline bool operator==(const vec4_T &rhs) {
            return (x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w);
    }

    inline bool operator!=(const vec4_T &rhs) {
            return (x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w);
    }

    inline vec4_T cross(const vec4_T& rhs) const {
            return vec4_T(data[1] * rhs.data[2] - data[2] * rhs.data[1],
                           data[2] * rhs.data[0] - data[0] * rhs.data[2],
                           data[0] * rhs.data[1] - data[1] * rhs.data[0], 0);
    }

    union {
        T data[4];
        struct {
            T x, y, z, w;
        };
    };
};
#undef vec4_T

typedef vec4<real_t> Vector4;
typedef vec4<int>    Vector4i;



#define N 4
struct Matrix4 {
    Matrix4(    real_t _00 = 0, real_t _01 = 0, real_t _02 = 0, real_t _03 = 0,
                real_t _10 = 0, real_t _11 = 0, real_t _12 = 0, real_t _13 = 0,
                real_t _20 = 0, real_t _21 = 0, real_t _22 = 0, real_t _23 = 0,
                real_t _30 = 0, real_t _31 = 0, real_t _32 = 0, real_t _33 = 0){
        //memcpy(data, &_00, sizeof(real_t) * 16);
            data[0] = _00;
            data[1] = _01;
            data[2] = _02;
            data[3] = _03;
            data[4] = _10;
            data[5] = _11;
            data[6] = _12;
            data[7] = _13;
            data[8] = _20;
            data[9] = _21;
            data[10] = _22;
            data[11] = _23;
            data[12] = _30;
            data[13] = _31;
            data[14] = _32;
            data[15] = _33;
    }

    Matrix4(const Matrix4 &copy) {
            memcpy(data, copy.data, sizeof(real_t) * 16);
    }


    Matrix4& operator=(const Matrix4 &rhs) {
            memcpy(data, rhs.data, sizeof(real_t) * 16);
            return *this;
    }

    static inline Matrix4 Identity(){
            return Matrix4(1, 0, 0, 0,
                           0, 1, 0, 0,
                           0, 0, 1, 0,
                           0, 0, 0, 1);
    }

    static inline Matrix4 Transpose(const Matrix4 &m) {
        return Matrix4(m.a, m.e, m.i, m.m,
                       m.b, m.f, m.j, m.n,
                       m.c, m.g, m.k, m.o,
                       m.d, m.h, m.l, m.p);
    }

    static inline Matrix4 Scale(const Vector3 &v) {
       return Matrix4(v.data[0], 0, 0, 0,
                        0, v.data[1], 0, 0,
                        0, 0, v.data[2], 0,
                        0, 0, 0, 1);
    }
    static inline Matrix4 Scale(const real_t x, const real_t y, const real_t z) {
       return Matrix4  (x, 0, 0, 0,
                        0, y, 0, 0,
                        0, 0, z, 0,
                        0, 0, 0, 1);
    }

    // @returns the translation matrix described by the vector
    static inline Matrix4 Translation(const Vector4 &v) {
       return Matrix4(1, 0, 0, v.data[0],
                        0, 1, 0, v.data[1],
                        0, 0, 1, v.data[2],
                        0, 0, 0, 1);
    }

    // @returns the rotation matrix about the x axis by the specified angle
    static inline Matrix4 RotationX(const real_t radians) {
       return Matrix4(1, 0, 0, 0,
                        0, cos(radians), -sin(radians), 0,
                        0, sin(radians), cos(radians), 0,
                        0, 0, 0, 1);
    }

    // @returns the rotation matrix about the y axis by the specified angle
    static inline Matrix4 RotationY(const real_t radians) {
       return Matrix4(cos(radians), 0, sin(radians), 0,
                        0, 1, 0, 0,
                        -sin(radians), 0, cos(radians), 0,
                        0, 0, 0, 1);
    }

    // @returns the rotation matrix about the z axis by the specified angle
    static inline Matrix4 RotationZ(const real_t radians) {
       return Matrix4(cos(radians), -sin(radians), 0, 0,
                        sin(radians), cos(radians), 0, 0,
                        0, 0, 1, 0,
                        0, 0, 0, 1);
    }

    // @returns the inverse scale matrix described by the vector
    static inline Matrix4 InvScale(const Vector4 &v) {
       return Matrix4(1 / v.data[0], 0, 0, 0,
                        0, 1 / v.data[1], 0, 0,
                        0, 0, 1 / v.data[2], 0,
                        0, 0, 0, 1);
    }

    // @returns the inverse translation matrix described by the vector
    static inline Matrix4 InvTranslate(const Vector4 &v) {
       return Matrix4(1, 0, 0, -v.data[0],
                        0, 1, 0, -v.data[1],
                        0, 0, 1, -v.data[2],
                        0, 0, 0, 1);
    }

    // @returns the inverse rotation matrix about the x axis by the specified angle
    static inline Matrix4 InvRotationX(const real_t radians) {
       return Transpose(RotationX(radians));
    }

    // @returns the inverse rotation matrix about the y axis by the specified angle
    static inline Matrix4 InvRotationY(const real_t radians) {
       return Transpose(RotationY(radians));
    }

    // @returns the inverse rotation matrix about the z axis by the specified angle
    static inline Matrix4 InvRotationZ(const real_t radians) {
       return Transpose(RotationZ(radians));
    }

    // @returns the inverse rotation matrix around the vector and point by the specified angle
    static inline Matrix4 InvRotation(const Vector4 &p, const Vector3 &d, const real_t n) {
       real_t theta = atan2(d.data[2], d.data[0]), phi = -atan2(d.data[1], sqrt(d.data[0] * d.data[0] + d.data[2] * d.data[2]));
       Matrix4 m = Translation(p);
       m *= InvRotationY(theta);
       m *= InvRotationZ(phi);
       m *= InvRotationX(n);
       m *= RotationZ(phi);
       m *= RotationY(theta);
       m *= InvTranslate(p);
       return m;
    }

    // @returns the rotation matrix around the vector and point by the specified angle
    static inline Matrix4 Rotation(const Vector4 &p, const Vector3 &d, const real_t n) {
       real_t theta = atan2(d.data[2], d.data[0]), phi = -atan2(d.data[1], sqrt(d.data[0] * d.data[0] + d.data[2] * d.data[2]));
       Matrix4 m = Translation(p);
       m *= InvRotationY(theta);
       m *= InvRotationZ(phi);
       m *= RotationX(n);
       m *= RotationZ(phi);
       m *= RotationY(theta);
       m *= InvTranslate(p);
       return m;
    }

    inline Matrix4& operator+=(const Matrix4& rhs) {
#pragma vector align
        for (unsigned i = 0; i < N*N; ++i)
            data[i] += rhs.data[i];
        return *this;
    }

    inline Matrix4& operator-=(const Matrix4& rhs) {
#pragma vector align
        for (unsigned i = 0; i < N*N; ++i)
            data[i] -= rhs.data[i];
        return *this;
    }

    inline Matrix4 operator-(const Matrix4 &rhs) {
            return Matrix4(*this) -= rhs;
    }

    inline Matrix4 operator+(const Matrix4 &rhs) {
            return Matrix4(*this) += rhs;
    }

    inline Matrix4& operator*=(const real_t rhs) {
            data[0] *= rhs;
            data[1] *= rhs;
            data[2] *= rhs;
            data[3] *= rhs;
            data[4] *= rhs;
            data[5] *= rhs;
            data[6] *= rhs;
            data[7] *= rhs;
            data[8] *= rhs;
            data[9] *= rhs;
            data[10] *= rhs;
            data[11] *= rhs;
            data[12] *= rhs;
            data[13] *= rhs;
            data[14] *= rhs;
            data[15] *= rhs;
            return *this;
    }

    inline Vector4 operator*(const Vector4 &rhs) const {
        return Vector4( a*rhs.x + b*rhs.y + c*rhs.z + d*rhs.w,
                        e*rhs.x + f*rhs.y + g*rhs.z + h*rhs.w,
                        i*rhs.x + j*rhs.y + k*rhs.z + l*rhs.w,
                        m*rhs.x + n*rhs.y + o*rhs.z + p*rhs.w );
    }

    inline void mulVec4(const Vector4 &rhs, Vector4 &out) {
        out.data[0] = a*rhs.x + b*rhs.y + c*rhs.z + d*rhs.w;
        out.data[1] = e*rhs.x + f*rhs.y + g*rhs.z + h*rhs.w;
        out.data[2] = i*rhs.x + j*rhs.y + k*rhs.z + l*rhs.w;
        out.data[3] = m*rhs.x + n*rhs.y + o*rhs.z + p*rhs.w;
    }

    inline Matrix4& operator/=(const real_t rhs) {
            data[0] /= rhs;
            data[1] /= rhs;
            data[2] /= rhs;
            data[3] /= rhs;
            data[4] /= rhs;
            data[5] /= rhs;
            data[6] /= rhs;
            data[7] /= rhs;
            data[8] /= rhs;
            data[9] /= rhs;
            data[10] /= rhs;
            data[11] /= rhs;
            data[12] /= rhs;
            data[13] /= rhs;
            data[14] /= rhs;
            data[15] /= rhs;
            return *this;
    }

    inline Matrix4 operator/(const real_t rhs) {
            return Matrix4(*this) /= rhs;
    }



    inline Matrix4 getInverse() const {
            return Matrix4(*this).invert();
    }

    inline real_t getDeterminant() {
        return a*f*k*p-a*f*l*o-a*g*j*p+a*g*l*n+a*h*j*o-a*h*k*n-b*e*k*p+b*e*l*o+b*g*i*p-b*g*l*m-b*h*i*o+b*h*k*m+c*e*j*p-c*e*l*n-c*f*i*p+c*f*l*m+c*h*i*n-c*h*j*m-d*e*j*o+d*e*k*n+d*f*i*o-d*f*k*m-d*g*i*n+d*g*j*m;
    }

    inline Matrix4& invert() {
         real_t det = getDeterminant(),
                _00 = (f*k*p+g*l*n+h*j*o-f*l*o-g*j*p-h*k*n)/det,
                _01 = (b*l*o+c*j*p+d*k*n-b*k*p-c*l*n-d*j*o)/det,
                _02 = (b*g*p+c*h*n+d*f*o-b*h*o-c*f*p-d*g*n)/det,
                _03 = (b*h*k+c*f*l+d*g*j-b*g*l-c*h*j-d*f*k)/det,
                _10 = (e*l*o+h*k*m+g*i*p-e*k*p-g*l*m-h*i*o)/det,
                _11 = (a*k*p+c*l*m+d*i*o-a*l*o-c*i*p-d*k*m)/det,
                _12 = (a*h*o+c*e*p+d*g*m-a*g*p-c*h*m-d*e*o)/det,
                _13 = (a*g*l+c*h*i+d*e*k-a*h*k-c*e*l-d*g*i)/det,
                _20 = (e*j*p+f*l*m+h*i*n-e*l*n-f*i*p-h*j*m)/det,
                _21 = (a*l*n+b*i*p+d*j*m-a*j*p-b*l*m-d*i*n)/det,
                _22 = (a*f*p+b*h*m+d*e*n-a*h*n-b*e*p-d*f*m)/det,
                _23 = (a*h*j+b*e*l+d*f*i-a*f*l-b*h*i-d*e*j)/det,
                _30 = (e*k*n+f*i*o+g*j*m-e*j*o-f*k*m-g*i*n)/det,
                _31 = (a*j*o+b*k*m+c*i*n-a*k*n-b*i*o-c*j*m)/det,
                _32 = (a*g*n+b*e*o+c*f*m-a*f*o-b*g*m-c*e*n)/det,
                _33 = (a*f*k+b*g*i+c*e*j-a*g*j-b*e*k-c*f*i)/det;
         a = _00;
         b = _01;
         c = _02;
         d = _03;
         e = _10;
         f = _11;
         g = _12;
         h = _13;
         i = _20;
         j = _21;
         k = _22;
         l = _23;
         m = _30;
         n = _31;
         o = _32;
         p = _33;
         return *this;
    }

    inline Matrix4& operator*=(const Matrix4 &rhs) {
            real_t _00 = data[0]  * rhs.data[0] + data[1]  * rhs.data[4] + data[2]  * rhs.data[8]  + data[3]  * rhs.data[12];
            real_t _01 = data[0]  * rhs.data[1] + data[1]  * rhs.data[5] + data[2]  * rhs.data[9]  + data[3]  * rhs.data[13];
            real_t _02 = data[0]  * rhs.data[2] + data[1]  * rhs.data[6] + data[2]  * rhs.data[10] + data[3]  * rhs.data[14];
            real_t _03 = data[0]  * rhs.data[3] + data[1]  * rhs.data[7] + data[2]  * rhs.data[11] + data[3]  * rhs.data[15];

            real_t _10 = data[4]  * rhs.data[0] + data[5]  * rhs.data[4] + data[6]  * rhs.data[8]  + data[7]  * rhs.data[12];
            real_t _11 = data[4]  * rhs.data[1] + data[5]  * rhs.data[5] + data[6]  * rhs.data[9]  + data[7]  * rhs.data[13];
            real_t _12 = data[4]  * rhs.data[2] + data[5]  * rhs.data[6] + data[6]  * rhs.data[10] + data[7]  * rhs.data[14];
            real_t _13 = data[4]  * rhs.data[3] + data[5]  * rhs.data[7] + data[6]  * rhs.data[11] + data[7]  * rhs.data[15];

            real_t _20 = data[8]  * rhs.data[0] + data[9]  * rhs.data[4] + data[10] * rhs.data[8]  + data[11] * rhs.data[12];
            real_t _21 = data[8]  * rhs.data[1] + data[9]  * rhs.data[5] + data[10] * rhs.data[9]  + data[11] * rhs.data[13];
            real_t _22 = data[8]  * rhs.data[2] + data[9]  * rhs.data[6] + data[10] * rhs.data[10] + data[11] * rhs.data[14];
            real_t _23 = data[8]  * rhs.data[3] + data[9]  * rhs.data[7] + data[10] * rhs.data[11] + data[11] * rhs.data[15];

            real_t _30 = data[12] * rhs.data[0] + data[13] * rhs.data[4] + data[14] * rhs.data[8]  + data[15] * rhs.data[12];
            real_t _31 = data[12] * rhs.data[1] + data[13] * rhs.data[5] + data[14] * rhs.data[9]  + data[15] * rhs.data[13];
            real_t _32 = data[12] * rhs.data[2] + data[13] * rhs.data[6] + data[14] * rhs.data[10] + data[15] * rhs.data[14];
            real_t _33 = data[12] * rhs.data[3] + data[13] * rhs.data[7] + data[14] * rhs.data[11] + data[15] * rhs.data[15];

            data[0] = _00;
            data[1] = _01;
            data[2] = _02;
            data[3] = _03;
            data[4] = _10;
            data[5] = _11;
            data[6] = _12;
            data[7] = _13;
            data[8] = _20;
            data[9] = _21;
            data[10] = _22;
            data[11] = _23;
            data[12] = _30;
            data[13] = _31;
            data[14] = _32;
            data[15] = _33;
            return *this;
    }

    inline Matrix4 operator*(const Matrix4& rhs) {
            return Matrix4(*this) *= rhs;
    }

    inline Matrix4 operator*(const real_t rhs) {
            return Matrix4(*this) *= rhs;
    }

    union {
        real_t data[N*N];
        struct {
            real_t a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p;
        };
    };
};

#undef N

//Global overloads

inline static real_t dot(const Vector3 &v1, const Vector3 &v2) {
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
inline static real_t dot(const Vector4 &v1, const Vector4 &v2) {
    assert(v1[3] == 0 || v2[3] == 0); // the 4th term should always zero out or we're doing something wrong
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline Vector3 operator*(const real_t scale, const Vector3 &rhs) {
    return Vector3(rhs.x * scale, rhs.y * scale, rhs.z * scale);
}
inline Vector3 operator-(const Vector3 &rhs) {
    return Vector3(-rhs.x, -rhs.y, -rhs.z);
}

inline Vector4 operator*(const real_t scale, const Vector4 &rhs) {
    return Vector4(rhs.x * scale, rhs.y * scale, rhs.z * scale, rhs.w * scale);
}

inline Vector4 operator-(const Vector4 &rhs) {
    return Vector4(-rhs.x, -rhs.y, -rhs.z, -rhs.w);
}

inline std::ostream& operator<<(std::ostream& os, const Matrix4& f) {
        for (unsigned i = 0; i < 16; ++i) {
            os << f.data[i] << (i % 4 == 3 ? "\n" : "\t");
        }
        return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vector4& f) {
        os <<"[";
        for (unsigned i = 0; i < 4; ++i) {
            os << f.data[i] << ",";
        }
        os << "]";
        return os;
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& f) {
        os <<"[";
        for (unsigned i = 0; i < 3; ++i) {
            os << f.data[i] << ",";
        }
        os << "]";
        return os;
}


#endif // PMATH_H
