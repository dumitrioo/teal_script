#pragma once

#include "../commondefs.hpp"
#include "vector4.hpp"
#include "matrix4.hpp"

namespace scfx::math {

    template<typename T, typename FUNC_T>
    static T df(FUNC_T const &f, T x, T dx = 1e-6) noexcept {
        T f0{f(x)};
        T f1{f(x + dx)};
        return (f1 - f0) / dx;
    }

#ifdef PLATFORM_WINDOWS
#if defined min
#undef min
#endif
#if defined max
#undef max
#endif
#endif

    template<typename T>
    T min(T v1, T v2) noexcept { return v1 < v2 ? v1 : v2; }

    template<typename T>
    T max(T v1, T v2) noexcept { return v2 < v1 ? v1 : v2; }

    template<typename T>
    T clamp(T value, T low, T hi) noexcept {
        if(value < low) { return low; }
        if(value > hi) { return hi; }
        return value;
    }

    template<typename T>
    T sign(T v) noexcept { return v < 0 ? -1 : 1; }

    template<typename T>
    bool negative(T x) noexcept {
        return x < 0;
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T sigmoid(T x, T xshift = 0, T yshift = 0, T slope = 1) noexcept {
        return static_cast<T>(1) / (static_cast<T>(1) + std::exp(-(x - xshift) * slope)) + yshift;
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T sigmoid_nonmodified(T x) noexcept {
        return static_cast<T>(1) / (static_cast<T>(1) + std::exp(-x));
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T tanh(T x) {
        T ex{std::exp(x)};
        T enx{std::exp(-x)};
        return (ex - enx)/(ex + enx);
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T relu(T x) noexcept {
        return (x + std::abs(x)) * 0.5;
    }

    template<typename T>
    T abs(T v) noexcept { return v < 0 ? -v : v; }

    template<typename T>
    T sqr(T v) noexcept { return v * v; }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T deg2rad(T d) noexcept { return d * M_PIl / 180.0L; }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T rad2deg(T r) noexcept { return r * 180.0L / M_PIl; }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T angle3d(const vector4<T> &u, const vector4<T> &v) {
        return std::acos((u & v) / (v.length() * u.length()));
    }

    template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    T angle_xy(const vector4<T> & u, const vector4<T> &v) noexcept {
        T ang1, ang2, ul = u.length(), vl = v.length();
        ang1 = std::acos(u.x() / ul);
        if(u.y() < 0.0) { ang1 = -ang1; }
        ang2 = std::acos(v.x() / vl);
        if(v.y() < 0.0) { ang2=-ang2; }
        return ang2 - ang1;
    }

    template<typename T>
    vector4<T> spheric_to_cartesian(T r, T theta, T phi) noexcept {
        vector4<T> res{0, 0, 0, 1};
        res.x() = r * std::sin(theta) * std::cos(phi);
        res.y() = r * std::sin(theta) * std::sin(phi);
        res.z() = r * std::cos(theta);
        return res;
    }

    template<typename T>
    vector4<T> cartesian_to_spheric(T x, T y, T z) noexcept {
        vector4<T> res{0, 0, 0, 1};
        T l{std::sqrt(sqr(x) + sqr(y) + sqr(z))};
        res.r() = l;
        res.theta() = std::acos(z / l);
        res.phi() = std::atan(y / x);
        return res;
    }

    template <typename T>
    int baricenter(const T *a, int as) noexcept {
        int l = 0;
        int r = as - 1;
        while(l + 1 < r) {
            int m = l + r; m /= 2;
            T lm = 0;
            T rm = 0;
            for(int i = 0; i < as; ++i) {
                lm += (i < m) ? (m - i) * a[i] : 0;
                rm += (i > m) ? (i - m) * a[i]: 0;
            }
            if(lm < rm) {
                l = m;
            } else if(lm > rm) {
                r = m;
            } else {
                return m;
            }
        }
        return l;
    }

    template<typename T>
    T mushroomcap(T width, T x) noexcept {
        return width > 0.0L ? std::cos((M_PIl * .5L) * x / width) : 0.0L;
    }


    template<typename T>
    void normalize(T &v) noexcept {
        if(!v.empty()) {
            auto vmax{v[0]};
            for(auto x: v) { vmax = vmax < x ? x : vmax; }
            if(vmax > 0) { for(auto &x: v) { x /= vmax; } }
        }
    }

    template<typename FP_T>
    FP_T gaussian(FP_T x, FP_T mat_exp = 0, FP_T std_dev = 1) noexcept {
        return std::exp(-sqr(x - mat_exp) / (2 * sqr(std_dev))) / (std_dev * std::sqrt(2 * M_PI));
    }

    template<typename FP_T>
    class norm_dist_rng {
    public:
        norm_dist_rng(FP_T mean, FP_T dis): mean_{mean}, dis_{dis}, nrd_{mean, std::abs(dis) / 6} {}
        FP_T gen() { return nrd_(rd_); }
        FP_T operator()() { return gen(); }

    private:
        FP_T mean_;
        FP_T dis_;
        std::random_device rd_{};
        std::normal_distribution<FP_T> nrd_;
    };

#if 0
    // from quake game (just for fun)
    static float one_div_sqrt(float x) noexcept {
        float x2{x * 0.5f};
        float y{x};
        std::int32_t i{*(std::int32_t *)&y};
        i = /*0x5f3759df*/ 0x5f3504f3 - (i >> 1);
        y = *(float *)&i;
        y = y * (1.5f - x2 * y * y);
        y = y * (1.5f - x2 * y * y);
        return y;
    }

    // intended for double numbers (for fun also)
    static double one_div_sqrt(double x) noexcept {
        double x2{x * 0.5};
        double y{x};
        std::int64_t i{*(std::int64_t *)&y};
        i = 0x5fe6a09e667f3bcdLL - (i >> 1);
        y = *(double *)&i;
        y = y * (1.5 - x2 * y * y);
        y = y * (1.5 - x2 * y * y);
        return y;
    }
#endif

    template<typename T>
    vector4<T> operator*(matrix4<T> const &m, vector4<T> const &v) {
        vector4<T> res;
        T resw = m.at(3, 0) * v.x() + m.at(3, 1) * v.y() + m.at(3, 2) * v.z() + m.at(3, 3) * v.w();
        T wrcpr{1};
        if(resw != 0) {
            wrcpr /= resw;
            res.w() = 1;
        } else {
            res.w() = 0;
        }
        res.x() = (m.at(0, 0) * v.x() + m.at(0, 1) * v.y() + m.at(0, 2) * v.z() + m.at(0, 3) * v.w()) * wrcpr;
        res.y() = (m.at(1, 0) * v.x() + m.at(1, 1) * v.y() + m.at(1, 2) * v.z() + m.at(1, 3) * v.w()) * wrcpr;
        res.z() = (m.at(2, 0) * v.x() + m.at(2, 1) * v.y() + m.at(2, 2) * v.z() + m.at(2, 3) * v.w()) * wrcpr;
        return res;
    }

    template<typename T>
    vector4<T> operator*(const vector4<T> &v, matrix4<T> const &mm) {
        vector4<T> res{v};
        res *= mm;
        return res;
    }

    template<typename T>
    matrix4<T> look_at_matrix(
        T eyeX,
        T eyeY,
        T eyeZ,
        T centerX,
        T centerY,
        T centerZ,
        T upX,
        T upY,
        T upZ
    ) {
        vector4<T> f{vector4<T>{centerX - eyeX, centerY - eyeY, centerZ - eyeZ}.normalized()};
        vector4<T> up{vector4<T>{upX, upY, upZ}.normalized()};
        vector4<T> s{f ^ up};
        vector4<T> u{s.normalized() ^ f};
        matrix4<T> m{
             s[0],  s[1],  s[2], 0,
             u[0],  u[1],  u[2], 0,
            -f[0], -f[1], -f[2], 0,
                0,     0,     0, 1,
        };
        return m * matrix4<T>::translate(-eyeX, -eyeY, -eyeZ);
    }

}
