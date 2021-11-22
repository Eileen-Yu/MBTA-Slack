1. what's the return type of curl_easy_init()?
  A: A pointer to a curl entity which is stored in heap memory.
  pointer in stack, curl in heap

https://github.com/curl/curl/blob/0942017162b064c223cc6111306bf06903f3262b/lib/url.c#L644

https://github.com/curl/curl/blob/76e047fc27b3a0b9e6d6d00cacf536e7b7c1b532/lib/easy.c#L289


2. pointer and local variable in stack

```c++
#include <iostream>
#include <stdlib.h>

template <typename T>
T* pv(T v) {
  T x = v;

  std::cout<<x<<"\n";
  std::cout<<&x<<"\n";

  return &x;
}

int main() {
  char *p1 = pv('a');
  int *p2 = pv(1);

  std::cout<<p1<<"\n";
  std::cout<<*p1<<"\n";

  std::cout<<p2<<"\n";
  std::cout<<*p2<<"\n";

}

```


