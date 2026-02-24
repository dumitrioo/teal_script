#pragma once

#include "../commondefs.hpp"

namespace scfx::math {

    template<typename T>
    class matrix4 {
    public:
        class row {
            friend class matrix4;
        public:
            T const &operator[](std::size_t c) const {
                if(c > 3) {
                    throw std::range_error{"index out of range"};
                }
                return m4_[c];
            }
            T &operator[](std::size_t c) {
                if(c > 3) {
                    throw std::range_error{"index out of range"};
                }
                return m4_[c];
            }
            T const *ptr() const { return m4_; }
            T *ptr() { return m4_; }

            void exchange(row &other) {
                std::swap(other.m4_[0], m4_[0]);
                std::swap(other.m4_[1], m4_[1]);
                std::swap(other.m4_[2], m4_[2]);
                std::swap(other.m4_[3], m4_[3]);
            }

        private:
            T *m4_{nullptr};
            row(T *m4): m4_{m4} {}
        };

        class const_row {
            friend class matrix4;
        public:
            T const *ptr() const { return m4_; }
            T const &operator[](std::size_t c) const {
                if(c > 3) {
                    throw std::range_error{"index out of range"};
                }
                return m4_[c];
            }

        private:
            T const *m4_{nullptr};
            const_row(T const *m4): m4_{m4} {}
        };

        matrix4(
            T m00 = 0, T m01 = 0, T m02 = 0, T m03 = 0,
            T m10 = 0, T m11 = 0, T m12 = 0, T m13 = 0,
            T m20 = 0, T m21 = 0, T m22 = 0, T m23 = 0,
            T m30 = 0, T m31 = 0, T m32 = 0, T m33 = 0
        ):
            m_{
                 m00, m01, m02, m03,
                 m10, m11, m12, m13,
                 m20, m21, m22, m23,
                 m30, m31, m32, m33
            }
        {
        }

        matrix4(T *md, std::size_t n):
            m_{md, md + n}
        {
        }

        T *data() & {
            return m_.data();
        }

        T const *data() const & {
            return m_.data();
        }

        T &at(std::size_t r, std::size_t c) & {
            if(r > 3 || c > 3) {
                throw std::range_error{"index out of range"};
            }
            return m_[r * 4 + c];
        }

        T const &at(std::size_t r, std::size_t c) const & {
            if(r > 3 || c > 3) {
                throw std::range_error{"index out of range"};
            }
            return m_[r * 4 + c];
        }

        const_row get_row(std::size_t r) const & {
            if(r > 3) {
                throw std::range_error{"index out of range"};
            }
            return const_row{data() + r * 4};
        }

        row get_row(std::size_t r) & {
            if(r > 3) {
                throw std::range_error{"index out of range"};
            }
            return row{data() + r * 4};
        }

        const_row operator[](std::size_t r) const & {
            return get_row(r);
        }

        row operator[](std::size_t r) & {
            return get_row(r);
        }

        T const &at_flat_index(std::size_t idx) const & {
            if(idx > 15) {
                throw std::range_error{"index out of range"};
            }
            return m_[idx];
        }

        T &at_flat_index(std::size_t idx) & {
            if(idx > 15) {
                throw std::range_error{"index out of range"};
            }
            return m_[idx];
        }

        matrix4 const &operator+() const {
            return *this;
        }

        matrix4 operator-() const {
            matrix4 m{};
            for(int i{0}; i < 16; ++i) {
                m.m_[i] = - m_[i];
            }
            return m;
        }

        matrix4 &operator*=(matrix4 const &a) {
            matrix4 atrans{a.transposed()};
            matrix4 res{};
            for(std::size_t i{0}; i < 4; i++) {
                for(std::size_t j{0}; j < 4; j++) {
                    T sum{0};
                    for(std::size_t k{0}; k < 4; k++) {
                        sum += at(i, k) * atrans.at(j, k);
                    }
                    res.at(i, j) = sum;
                }
            }
            m_ = res.m_;
            return *this;
        }

        matrix4 &operator*=(T v) {
            for(std::size_t i{0}; i < 16; i++) {
                m_[i] *= v;
            }
            return *this;
        }

        void become_zero() { for(auto &&x: m_) { x = 0; } }

        T determinant() const { return det(*this); }

        matrix4 &become_identity() {
            m_[0]  = 1; m_[1] =  0; m_[2] =  0; m_[3] =  0;
            m_[4]  = 0; m_[5] =  1; m_[6] =  0; m_[7] =  0;
            m_[8]  = 0; m_[9] =  0; m_[10] = 1; m_[11] = 0;
            m_[12] = 0; m_[13] = 0; m_[14] = 0; m_[15] = 1;
            return *this;
        }

        matrix4 transposed() const {
            matrix4 res{};
            for(std::size_t i{0}; i < 4; ++i) {
                for(std::size_t j{0}; j < 4; ++j) {
                    res.at(i, j) = at(j, i);
                }
            }
            return res;
        }

        matrix4 inverted() const {
            T dtrm{det(*this)};
            if(dtrm != 0) {
                return adjugate() * (1 / dtrm);
            }
            return {};
        }

        friend matrix4 operator+(const matrix4& a, const matrix4& b) {
            matrix4 res{};
            for(std::size_t i{0}; i < 16; i++) {
                res.m_[i] = a.m_[i] + b.m_[i];
            }
            return res;
        }

        friend matrix4 operator-(matrix4 const &a, matrix4 const &b) {
            matrix4 res{};
            for(std::size_t i{0}; i < 16; i++) {
                res.m_[i] = a.m_[i] - b.m_[i];
            }
            return res;
        }

        friend matrix4 operator*(matrix4 const &a, matrix4 const &b) {
            matrix4 res{a};
            res *= b;
            return res;
        }

        friend matrix4 operator*(matrix4 const &a, T v) {
            matrix4 res{a};
            res *= v;
            return res;
        }

        friend matrix4 operator*(T v, matrix4 const &a) {
            matrix4 res{a};
            res *= v;
            return res;
        }

        friend matrix4 operator/(matrix4 const &a, T v) {
            matrix4 res{a};
            res /= v;
            return res;
        }

        friend bool operator==(matrix4 const &a, matrix4 const &b) {
            for(std::size_t i{0}; i < 16; i++) {
                if(a.m_[i] != b.m_[i]) { return false; }
            }
            return true;
        }

        friend bool operator!=(matrix4 const &a, matrix4 const &b) {
            for(std::size_t i{0}; i < 16; i++) {
                if(a.m_[i] != b.m_[i]) { return true; }
            }
            return false;
        }

        static matrix4 identity() {
            return matrix4{
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1,
            };
        }

        static matrix4 frustum(T left, T right, T bottom, T top, T znear, T zfar) {
            matrix4 res{};
            res[0][0] = (static_cast<T>(2) * znear) / (right - left);
            res[0][2] = (right + left) / (right - left);
            res[1][1] = (static_cast<T>(2) * znear) / (top - bottom);
            res[1][2] = (top + bottom) / (top - bottom);
            res[2][2] = (zfar + znear) / (zfar - znear);
            res[2][3] = (static_cast<T>(2) * zfar * znear) / (zfar - znear);
            res[3][2] = static_cast<T>(-1);
            return res;
        }

        static matrix4 perspective(T fov_y_degrees, T aspect_ratio, T znear, T zfar) {
            matrix4 res{};
            T f = static_cast<T>(1) / std::tan((fov_y_degrees * static_cast<T>(M_PI) / static_cast<T>(180)) / static_cast<T>(2));
            res[0][0] = f / aspect_ratio;
            res[1][1] = f;
            res[2][2] = (zfar + znear) / (znear - zfar);
            res[2][3] = (static_cast<T>(2) * zfar * znear) / (znear - zfar);
            res[3][2] = static_cast<T>(-1);
            return res;
        }


        static matrix4 translate(T x, T y, T z) {
            matrix4 res{identity()};
            res.at(0, 3) = x;
            res.at(1, 3) = y;
            res.at(2, 3) = z;
            return res;
        }

        static matrix4 scale(T x, T y, T z) {
            matrix4 res{identity()};
            res.at(0, 0) = x;
            res.at(1, 1) = y;
            res.at(2, 2) = z;
            return res;
        }

        static matrix4 rotate_x(T angle) {
            matrix4 res{identity()};
            T cosine = std::cos(angle);
            T sine = std::sin(angle);
            res.at(1, 1) = cosine;
            res.at(1, 2) = -sine;
            res.at(2, 1) = sine;
            res.at(2, 2) = cosine;
            return res;
        }

        static matrix4 rotate_y(T angle) {
            matrix4 res{identity()};
            T cosine = std::cos(angle);
            T sine = std::sin(angle);
            res.at(0, 0) = cosine;
            res.at(0, 2) = sine;
            res.at(2, 0) = -sine;
            res.at(2, 2) = cosine;
            return res;
        }

        static matrix4 rotate_z(T angle) {
            matrix4 res{identity()};
            T cosine = std::cos(angle);
            T sine = std::sin(angle);
            res.at(0, 0) = cosine;
            res.at(0, 1) = -sine;
            res.at(1, 0) = sine;
            res.at(1, 1) = cosine;
            return res;
        }

        static matrix4 rotation(T angle, T x, T y, T z) {
            matrix4 res{identity()};
            if(!std::isnormal(angle)) {
                return res;
            }
            T l{std::sqrt(x * x + y * y + z * z)};
            if(std::isnormal(l)) {
                if(l != 1) { x /= l; y /= l; z /= l; }

                T c{std::cos(angle)};
                T s{std::sin(angle)};
#if 0
                row r0{res.get_row(0)};
                r0[0] = x * x + (1 - x * x) * c;
                r0[1] = x * y * (1 - c) - z * s;
                r0[2] = x * z * (1 - c) + y * s;
                r0[3] = 0;
                row r1{res.get_row(1)};
                r1[0] = x * y * (1 - c) + z * s;
                r1[1] = y * y + (1 - y * y) * c;
                r1[2] = y * z * (1 - c) - x * s;
                r1[3] = 0;
                row r2{res.get_row(2)};
                r2[0] = x * z * (1 - c) - y * s;
                r2[1] = y * z * (1 - c) + x * s;
                r2[2] = z * z + (1 - z * z) * c;
                r2[3] = 0;
                row r3{res.get_row(3)};
                r3[0] = 0; r3[1] = 0; r3[2] = 0; r3[3] = 1;
#else
                res[0][0] = c + (1 - c) * x * x;
                res[0][1] = (1 - c) * x * y - s * z;
                res[0][2] = (1 - c) * x * z + s * y;
                res[0][3] = 0;
                res[1][0] = (1 - c) * x * y + s * z;
                res[1][1] = c + (1 - c) * y * y;
                res[1][2] = (1 - c) * y * z - s * x;
                res[1][3] = 0;
                res[2][0] = (1 - c) * x * z - s * y;
                res[2][1] = (1 - c) * y * z + s * x;
                res[2][2] = c + (1 - c) * z * z;
                res[2][3] = 0;
                res[3][0] = 0;
                res[3][1] = 0;
                res[3][2] = 0;
                res[3][3] = 1;
#endif
            }
            return res;
        }

        static matrix4 mirror_x() {
            matrix4 res{identity()};
            res.at(0, 0) = -1;
            return res;
        }

        static matrix4 mirror_y() {
            matrix4 res{identity()};
            res.at(1, 1) = -1;
            return res;
        }

        static matrix4 mirror_z() {
            matrix4 res{identity()};
            res.at(2, 2) = -1;
            return res;
        }

    private:
        matrix4 submatrix3(std::size_t r, std::size_t c) const {
            matrix4 res{};
            std::size_t subi = 0;
            for(std::size_t i = 0; i < 4; i++) {
                if(i == r) {
                    continue;
                }
                std::size_t subj = 0;
                for(std::size_t j = 0; j < 4; j++) {
                    if(j == c) {
                        continue;
                    }
                    res.at(subi, subj++) = at(i, j);
                }
                subi++;
            }
            return res;
        }

        matrix4 adjugate() const {
            matrix4 res{};
            for(std::size_t r = 0; r < 4; ++r) {
                for(std::size_t c = 0; c < 4; ++c) {
                    res.at(r, c) = det(submatrix3(c, r), 3) * ((r + c) % 2 == 0 ? 1 : -1);
                }
            }
            return res;
        }

        static T det(matrix4 const &mat, std::size_t n = 4) {
            if(n == 2) {
                return mat.at(0, 0) * mat.at(1, 1) - mat.at(0, 1) * mat.at(1, 0);
            } else {
                T d{};
                for(std::size_t c = 0; c < n; c++) {
                    matrix4 submat{};
                    std::size_t subi = 0;
                    for(std::size_t i = 1; i < n; i++) {
                        std::size_t subj = 0;
                        for(std::size_t j = 0; j < n; j++) {
                            if(j == c) {
                                continue;
                            }
                            submat.at(subi, subj++) = mat.at(i, j);
                        }
                        subi++;
                    }
                    d += mat.at(0, c) * det(submat, n - 1) * (c % 2 == 0 ? 1 : -1);
                 }
                return d;
             }
        }

    private:
        std::array<T, 16> m_{
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
            0, 0, 0, 0,
        };
    };

    template<typename T>
    static std::ostream &operator<<(std::ostream &os, const matrix4<T> &m) {
        std::stringstream ss{};
        ss << "{";
        std::string sep1{};
        for(std::size_t r{0}; r < 4; ++r) {
             ss << sep1 << "{";
             std::string sep2{};
             for(std::size_t c{0}; c < 4; ++c) {
                ss << sep2 << std::fixed << m.at(r, c);
                sep2 = ", ";
             }
             ss << "}";
             if(sep1.empty()) {
                 sep1 = ", ";
             }
        }
        ss << "}";
        os << ss.str();
        return os;
    }

}
