#ifndef ATSCOPEEXIT_H
#define ATSCOPEEXIT_H

#include <utility>

/*!
 * \brief The AtScopeExit class provides a facility to execute a callable at scope exit
 * If you want to use a member function as callable, use std::bind
 * \code
 * {
 *   auto e1 = createScopeExitGuard([]{doSomeCleanupHere();});
 *   auto e2 = createScopeExitGuard(doSomeCleanupHere);
 *   auto e3 = createScopeExitGuard(std::bind(&MyClass::cleanup, this));
 *   auto e4 = createScopeExitGuard(MyFunctor{});
 * }
 * \endcode
 */
template<typename Callable>
class AtScopeExit
{
public:
    AtScopeExit(Callable callable)
        : callee(callable)
        , valid{true}
    {
    }

    ~AtScopeExit()
    {
        if (valid)
            callee();
    }

    AtScopeExit(const AtScopeExit &other) = delete;
    AtScopeExit &operator=(const AtScopeExit &other) = delete;
    AtScopeExit(AtScopeExit &&other)
        : callee{std::move(other.callee)}
    {
        other.valid = false;
    }

    AtScopeExit &operator=(AtScopeExit  &&other)
    {
        if (this != &other)
        {
            callee = std::move(other.callee);
            valid = true;
            other.valid = false;
        }
        return *this;
    }

private:
    Callable callee;
    bool valid;
};

template<typename Callable>
AtScopeExit<Callable> createScopeExitGuard(Callable callable)
{
    return AtScopeExit<Callable>(callable);
}

#endif
