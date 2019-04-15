#ifndef LAMBDA_HPP
#define LAMBDA_HPP

#include "object.hpp"

namespace BmCpp {
template <typename T>
class Lambda;

template <typename ReturnValue, typename... Args>
class Lambda<ReturnValue(Args...)> {
public:
    template <typename T>
    Lambda(T t) : callable_(new CallableT<T>(t)) {
    }

    template <typename T>
    Lambda& operator=(T t) {
        callable_ = new CallableT<T>(t);
        return *this;
    }

    ReturnValue operator()(Args... args) const {
        assert(callable_);
        return callable_->Invoke(args...);
    }

private:
    class ICallable : public BmCpp::Object {
    public:
        MAKE_ALL_PTRS(ICallable);
        virtual ~ICallable() = default;
        virtual ReturnValue Invoke(Args...) = 0;
    };

    template <typename T>
    class CallableT : public ICallable {
    public:
        CallableT(const T& t)
            : t_(t) {
        }

        ~CallableT() override = default;

        ReturnValue Invoke(Args... args) override {
            return t_(args...);
        }

    private:
        T t_;
    };

    typename ICallable::OPtr callable_;
};
}
#endif // LAMBDA_HPP
