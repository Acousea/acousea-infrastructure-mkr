#ifndef CURVES_H
#define CURVES_H


/**
 * f(x) = a*x + b
 * f^-1(y) = (y - b) / a
 */
class LinearCurve final
{
    double a, b; // y = a*x + b
public:
    LinearCurve(const double slope, const double intercept) : a(slope), b(intercept)
    {
    }

    [[nodiscard]] double forward(const double x) const { return a * x + b; }
    [[nodiscard]] double inverse(const double y) const { return (y - b) / a; }
};

#include <cmath>

/**
 * f(t) = vmin + (vmax - vmin) / (1 + exp(k * (t - t0)))
 * f^-1(v) = t0 + ln((vmax - vmin) / (v - vmin) - 1) / k
 */
class LogisticCurve final
{
    double vmin, vmax, k, t0;

public:
    LogisticCurve(const double vmin_, const double vmax_, const double k_, const double t0_)
        : vmin(vmin_), vmax(vmax_), k(k_), t0(t0_)
    {
    }

    [[nodiscard]] double forward(const double t) const
    {
        return vmin + (vmax - vmin) / (1.0 + std::exp(k * (t - t0)));
    }

    [[nodiscard]] double inverse(const double v) const
    {
        // invertir la función logística
        const double ratio = (vmax - vmin) / (v - vmin) - 1.0;
        return t0 + std::log(ratio) / k;
    }
};

#endif //CURVES_H
