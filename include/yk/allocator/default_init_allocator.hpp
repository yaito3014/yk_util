#ifndef YK_UTIL_ALLOCATOR_DEFAULT_INIT_ALLOCATOR_HPP
#define YK_UTIL_ALLOCATOR_DEFAULT_INIT_ALLOCATOR_HPP

#include "yk/allocator/concepts.hpp"

#include <memory>
#include <utility>
#include <new>
#include <type_traits>


namespace yk {

// http://stackoverflow.com/a/21028912/273767

template<class T, class A = std::allocator<T>>
class default_init_allocator : public A
{
    using traits = std::allocator_traits<A>;

public:
    template<class U>
    struct rebind
    {
        using other = default_init_allocator<U, typename traits::template rebind_alloc<U>>;
    };

    using A::A;

    template <typename U>
    void construct(U* ptr) noexcept(std::is_nothrow_default_constructible_v<U>)
    {
        ::new(static_cast<void*>(ptr)) U;
    }
    template <typename U, typename...Args>
    void construct(U* ptr, Args&&... args)
    {
        traits::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
    }
};

}

#endif
