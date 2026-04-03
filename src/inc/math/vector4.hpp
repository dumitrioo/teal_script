#pragma once

#include "../commondefs.hpp"
#include "matrix4.hpp"

namespace teal::math {

    template<typename T>
    class vector4 {
    public:
        vector4(T px = 0, T py = 0, T pz = 0, T pw = 0):
            v_{px, py, pz, pw}
        {
        }

        T &x() & {
            return v_[0];
        }
        T const &x() const & {
            return v_[0];
        }

        T &y() & {
            return v_[1];
        }
        T const &y() const & {
            return v_[1];
        }

        T &z() & {
            return v_[2];
        }
        T const &z() const & {
            return v_[2];
        }

        T &w() & {
            return v_[3];
        }
        T const &w() const & {
            return v_[3];
        }

        // spherical coords - radial distance from original
        T &r() & {
            return v_[0];
        }
        T const &r() const & {
            return v_[0];
        }

        // spherical coords - polar angle (between ray and Z axis)
        T &theta() & {
            return v_[1];
        }
        T const &theta() const & {
            return v_[1];
        }

        // spherical coords - azimuthal angle (between ray and X axis, inside the XY plain)
        T &phi() & {
            return v_[2];
        }
        T const &phi() const & {
            return v_[2];
        }

        T &operator[](std::size_t index) & {
            if(index > 3) {
                throw std::range_error{"index out of range"};
            }
            return v_[index];
        }

        T const &operator[](std::size_t index) const & {
            if(index > 3) {
                throw std::range_error{"index out of range"};
            }
            return v_[index];
        }

        void reset(T v = T{}) { for(auto &&x: v_) { x = v; } }

        void from_spherical(T r, T plr_angle, T azmth_angle) {
            v_[0] = r * std::sin(plr_angle) * std::cos(azmth_angle);
            v_[1] = r * std::sin(plr_angle) * std::sin(azmth_angle);
            v_[2] = r * std::cos(plr_angle);
        }

        T length() const {
            return std::sqrt(v_[0] * v_[0] + v_[1] * v_[1] + v_[2] * v_[2]);
        }

        T polar_angle() const {
            T l{length()};
            if(l > 0) {
                return std::acos(v_[2] / l);
            }
            return 0;
        }

        T azimuth_angle() const {
            T l{length()};
            if(l > 0 && v_[0] != 0) {
                return std::atan(v_[1] / v_[0]);
            }
            return 0;
        }

        std::vector<T> as_stlvect4() const {
            return std::vector<T>{v_.begin(), v_.end()};
        }

        std::vector<T> as_stlvect3() const {
            return std::vector<T>{v_.begin(), v_.begin() + 3};
        }

        std::vector<T> as_stlvect2() const {
            return std::vector<T>{v_.begin(), v_.begin() + 2};
        }

        vector4 const &operator+() const {
            return *this;
        }

        vector4 operator-() const {
            return vector4{-v_[0], -v_[1], -v_[2], v_[3]};
        }

        vector4 &operator+=(vector4 const &v) {
            v_[0] += v.v_[0];
            v_[1] += v.v_[1];
            v_[2] += v.v_[2];

            return *this;
        }

        vector4 &operator-=(vector4 const &v) {
            v_[0] -= v.v_[0];
            v_[1] -= v.v_[1];
            v_[2] -= v.v_[2];

            return *this;
        }

        vector4 &operator*=(T f) {
            v_[0] *= f;
            v_[1] *= f;
            v_[2] *= f;

            return *this;
        }

        vector4 &operator*=(matrix4<T> const &matr) {
            matrix4<T> m{matr.transposed()};
            vector4 res{};

            T resw = m.at(3, 0) * x() + m.at(3, 1) * y() + m.at(3, 2) * z() + m.at(3, 3) * w();
            T wrcpr{1};
            if(resw != 0) {
                wrcpr /= resw;
                res.w() = 1;
            } else {
                res.w() = 0;
            }
            res.v_[0] = m.at(0, 0) * v_[0] + m.at(0, 1) * v_[1] + m.at(0, 2) * v_[2] + m.at(0, 3) * v_[3];
            res.v_[1] = m.at(1, 0) * v_[0] + m.at(1, 1) * v_[1] + m.at(1, 2) * v_[2] + m.at(1, 3) * v_[3];
            res.v_[2] = m.at(2, 0) * v_[0] + m.at(2, 1) * v_[1] + m.at(2, 2) * v_[2] + m.at(2, 3) * v_[3];
            *this = res;
            return *this;
        }

        vector4 &operator/=(T f) {
            v_[0] /= f;
            v_[1] /= f;
            v_[2] /= f;

            return *this;
        }

        int	operator==(vector4 const &v) const {
            return v_[0] == v.v_[0] && v_[1] == v.v_[1] && v_[2] == v.v_[2] && v_[3] == v.v_[3];
        }

        int	operator!=(vector4 const &v) const {
            return v_[0] != v.v_[0] || v_[1] != v.v_[1] || v_[2] != v.v_[2] || v_[3] != v.v_[3];
        }

        void become_zero() {
            for(auto &&x: v_) {
                x = 0;
            }
        }

        vector4 &normalize() {
            T l{length()};
            if(l > 0) {
                this->operator /= (l);
            }
            return *this;
        }

        vector4 normalized() const {
            return vector4{*this}.normalize();
        }

        bool is_zero() const {
            return v_[0] == 0 && v_[1] == 0 && v_[2] == 0 && v_[3] == 0;
        }

        vector4 all_positive() const {
            vector4 res{};
            res.v_[0] = std::abs(v_[0]);
            res.v_[1] = std::abs(v_[1]);
            res.v_[2] = std::abs(v_[2]);
            res.v_[3] = std::abs(v_[3]);
            return res;
        }

        friend vector4 operator+(vector4 const &u, vector4 const &v) {
            return vector4{u.v_[0] + v.v_[0], u.v_[1] + v.v_[1], u.v_[2] + v.v_[2], u.v_[3] + v.v_[3]};
        }

        friend vector4 operator-(vector4 const &u, vector4 const &v) {
            return vector4{u.v_[0] - v.v_[0], u.v_[1] - v.v_[1], u.v_[2] - v.v_[2], u.v_[3] - v.v_[3]};
        }

        friend T operator*(vector4 const &u, vector4 const &v) {
            return u.v_[0] * v.v_[0] + u.v_[1] * v.v_[1] + u.v_[2] * v.v_[2];
        }

        friend vector4 operator*(vector4 const &v, T a) {
            return vector4{v.v_[0] * a, v.v_[1] * a, v.v_[2] * a};
        }

        friend vector4 operator*(T a, vector4 const &v) {
            return vector4{v.v_[0] * a, v.v_[1] * a, v.v_[2] * a};
        }

        friend vector4 operator/(vector4 const &v, T a) {
            return vector4{v.v_[0] / a, v.v_[1] / a, v.v_[2] / a};
        }

        friend T operator&(vector4 const &u, vector4 const &v) {
            return u.v_[0] * v.v_[0] + u.v_[1] * v.v_[1] + u.v_[2] * v.v_[2];
        }

        friend vector4 operator^(vector4 const &u, vector4 const &v) {
            return vector4{u.v_[1] * v.v_[2] - u.v_[2] * v.v_[1], u.v_[2] * v.v_[0] - u.v_[0] * v.v_[2], u.v_[0] * v.v_[1] - u.v_[1] * v.v_[0]};
        }

    public:
        std::array<T, 4> v_{};
    };

}

template<typename T>
static std::ostream &operator<<(std::ostream &os, teal::math::vector4<T> const &v) {
    std::stringstream ss{};
    ss << "{";
    std::string sep{};
    for(std::size_t i{0}; i < 4; ++i) { ss << sep << std::fixed << v[i]; if(sep.empty()) { sep = ", "; } }
    ss << "}";
    os << ss.str();
    return os;
}
