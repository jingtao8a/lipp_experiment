//
// Created by 19210 on 2023/12/15.
//

#ifndef LIPP_LINEAR_REGRESSION_MODEL_H
#define LIPP_LINEAR_REGRESSION_MODEL_H

#include <algorithm>

/*** Linear model and model builder ***/

// Forward declaration
template <class T>
class LinearModelBuilder;

// Linear regression model
template <class T>
class LinearModel {
public:
    double a_ = 0;  // slope
    double b_ = 0;  // intercept

    LinearModel() = default;
    LinearModel(double a, double b) : a_(a), b_(b) {}
    explicit LinearModel(const LinearModel& other) : a_(other.a_), b_(other.b_) {}

    void expand(double expansion_factor) {
        a_ *= expansion_factor;
        b_ *= expansion_factor;
    }

    inline int predict(T key) const {
        return static_cast<int>(a_ * static_cast<double>(key) + b_);
    }

    inline double predict_double(T key) const {
        return a_ * static_cast<double>(key) + b_;
    }
};

template <class T>
class LinearModelBuilder {
public:
    LinearModel<T>* model_;

    explicit LinearModelBuilder<T>(LinearModel<T>* model) : model_(model) {}

    inline void add(T x, int y) {
        count_++;
        x_sum_ += static_cast<long double>(x);
        y_sum_ += static_cast<long double>(y);
        xx_sum_ += static_cast<long double>(x) * x;
        xy_sum_ += static_cast<long double>(x) * y;
        x_min_ = std::min<T>(x, x_min_);
        x_max_ = std::max<T>(x, x_max_);
        y_min_ = std::min<double>(y, y_min_);
        y_max_ = std::max<double>(y, y_max_);
    }

    void build() {
        if (count_ <= 1) {
            model_->a_ = 0;
            model_->b_ = static_cast<double>(y_sum_);
            return;
        }

        if (static_cast<long double>(count_) * xx_sum_ - x_sum_ * x_sum_ == 0) {
            // all values in a bucket have the same key.
            model_->a_ = 0;
            model_->b_ = static_cast<double>(y_sum_) / count_;
            return;
        }

        auto slope = static_cast<double>(
                (static_cast<long double>(count_) * xy_sum_ - x_sum_ * y_sum_) /
                (static_cast<long double>(count_) * xx_sum_ - x_sum_ * x_sum_));
        auto intercept = static_cast<double>(
                (y_sum_ - static_cast<long double>(slope) * x_sum_) / count_);
        model_->a_ = slope;
        model_->b_ = intercept;

        // If floating point precision errors, fit spline
        if (model_->a_ <= 0) {
            model_->a_ = (y_max_ - y_min_) / (x_max_ - x_min_);
            model_->b_ = -static_cast<double>(x_min_) * model_->a_;
        }
    }

private:
    int count_ = 0;
    long double x_sum_ = 0;
    long double y_sum_ = 0;
    long double xx_sum_ = 0;
    long double xy_sum_ = 0;
    T x_min_ = std::numeric_limits<T>::max();
    T x_max_ = std::numeric_limits<T>::lowest();
    double y_min_ = std::numeric_limits<double>::max();
    double y_max_ = std::numeric_limits<double>::lowest();
};


#endif //LIPP_LINEAR_REGRESSION_MODEL_H
