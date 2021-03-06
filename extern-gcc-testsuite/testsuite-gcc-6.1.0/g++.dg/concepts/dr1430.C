// PR c++/66092
// { dg-options "-std=c++1z -fconcepts" }

#include <type_traits>

template <typename T, typename U, typename... Args>
requires (sizeof...(Args) == 0)
  constexpr decltype(auto) check()
  {
    return std::integral_constant<bool, __is_same_as(T, U)>();
  }

template <typename T, typename U, typename... Args>
requires (sizeof...(Args) > 0)
  constexpr decltype(auto) check()
  {
    return std::integral_constant<bool, __is_same_as(T, U)
        && decltype(check<U, Args...>())::value>();
  }

template <typename T, typename U, typename... Args>
  concept bool Same()
  {
    return decltype(check<T, U, Args...>())::value;
  }

template <typename... Args>
requires Same<Args...>()	// { dg-error "concept" }
  void foo( Args... args ) {}

int main()
{
  foo(1, 2, 3);			// { dg-error "" }
}
