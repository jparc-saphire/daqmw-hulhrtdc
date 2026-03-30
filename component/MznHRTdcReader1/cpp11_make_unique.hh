#ifndef cpp11_make_unique_h
#define cpp11_make_unique_h

#if __cplusplus < 201402L
#include <memory>

//______________________________________________________________________________
template <typename T, typename ... Args>
std::unique_ptr<T> make_unique(Args&& ... args)
{
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif


#endif
