/*
 *  Copyright (c) 2021-2024 Victor M. Barrientos (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
 * 
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */

#ifndef CDP_UTIL_H
#define CDP_UTIL_H


#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>


#define     CDPPASTIT(a,b)            a##b
#define     CDPPASTE(a,b)             CDPPASTIT(a, b)
#define     CDP(_, i)                 CDPPASTE(__##i, _)

#define     CDP_EXPECT(exp)           (__builtin_expect((long)(exp), true))
#define     CDP_EXPECT_PTR(p)         CDP_EXPECT((p) != NULL)
#define     CDP_RARELY(exp)           (__builtin_expect((long)(exp), false))
#define     CDP_RARELY_PTR(p)         CDP_RARELY((p) != NULL)


/*
 * Variable Initialization
 */

#define     CDP_T(v, x, ...)          __auto_type (x) __VA_ARGS__ = (v)
#define     CDP_U(_, x, v, ...)       CDP_T(v, CDP(_,x), ##__VA_ARGS__)
#define     CDP_I(T, p, a, ...)       T* (p) __VA_ARGS__ = (T*) (a)

#define     CDP_TLS   __thread



/*
 * Memory Initialization
 */

#if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
  #error    unsoported target platform!
#else
  static inline void*   cdp_malloc(size_t z)                   {void* p = malloc(z);       if CDP_RARELY(!p)  abort();  return p;}
  static inline void*   cdp_calloc(size_t n, size_t z)         {void* p = calloc(n, z);    if CDP_RARELY(!p)  abort();  return p;}
  static inline void*   cdp_realloc(void* p, size_t z)         {void* r = realloc(p, z);   if CDP_RARELY(!r)  abort();  return r;}
  #define   cdp_alloca  __builtin_alloca
  #define   cdp_free    free
#endif
#define     cdp_malloc0(z, ...)       cdp_calloc(1, z __VA_ARGS__)


#define     cdp_new(T, ...)           cdp_malloc0(sizeof(T) __VA_ARGS__)
#define     CDP_NEW(T, p, ...)        CDP_I(T, p, cdp_new(T, ##__VA_ARGS__))
#define     CDP_REALLOC(p, z)         ({(p) = cdp_realloc(p, z);})
#define     CDP_FREE(p)               do{ cdp_free(p); (p) = NULL; }while (0)
#define     CDP_AUTOFREE(T, p, ...)   T* (p) __attribute__((cleanup(cdp_free))) __VA_ARGS__


static inline void  cdp_cpy_or_0(void* q, void* p, size_t z)  {assert(q);  if (p) memcpy(q, p, z); else memset(q, 0, z);}

#define     CDP_CLONE(T, d, p)        T* (d) = cdp_clone(p, sizeof(T))
#define     CDP_0(p)                  memset(p, 0, sizeof(*(p)))
#define     _CDP_SWAP(_, a, b)        do{ CDP_U(_,c, a); (a) = (b); (b) = CDP(_,c); }while(0)
#define     CDP_SWAP(...)             _CDP_SWAP(__COUNTER__, __VA_ARGS__)

typedef void (*cdpDel)(void*);



/*
 * Pointer Utilities
 */

#define     CDP_P(p)                  ((void*)(p))
#define     cdp_v2p(v)                ((void*)(uintptr_t)(v))
#define     cdp_p2v(p)                ((uintptr_t)(p))

#define     _cdp_align_to(_, u, _a)   ({CDP_U(_,a, _a);  assert(0 < CDP(_,a));  ((u) + (CDP(_,a) - 1)) & ~(CDP(_,a) - 1);})
#define     cdp_align_to(...)         _cdp_align_to(__COUNTER__, __VA_ARGS__)
#define     cdp_align_max(u)          cdp_align_to(u, __BIGGEST_ALIGNMENT__)
#define     cdp_aligned(t)            cdp_align_to(sizeof(t), __alignof__(t))
#define     CDP_ALIGN_TO(u, a)        ((u) = cdp_align_to(u, a))

#define     cdp_ptr_align_to(p, a)    ((void*)cdp_align_to((uintptr_t)(p), a))
#define     cdp_ptr_aligned(p)        ((void*)cdp_aligned((uintptr_t)(p)))
#define     cdp_ptr_off(p, off)       ((void*)(((uint8_t*)(p)) + (off)))
#define     CDP_PTR_OFF(p, off)       ((p) = cdp_ptr_off(p, off))
#define     cdp_ptr_dif(p1, p2)       ((void*)(((uint8_t*)(p1)) - ((uint8_t*)(p2))))
#define     cdp_ptr_idx(p, o, z)      ((size_t)cdp_ptr_dif(o, p) / (z))
#define     cdp_ptr_adr(p, i, z)      cdp_ptr_off(p, (i)*(z))
#define     cdp_ptr_sec_get(p, v)     ((p)? *(p): (n))
#define     CDP_PTR_SEC_SET(p, n)     ({if (p) *(p)=(n);})
#define     CDP_PTR_OVERW(p, n)       ({cdp_free(p); (p)=(n);})


#define     cdp_popcount(v)           __builtin_choose_expr(sizeof(v) <= sizeof(int), __builtin_popcount(v), __builtin_choose_expr(sizeof(v) == sizeof(long int), __builtin_popcountl(v), __builtin_popcountll(v)))
#define     cdp_clz(v)                __builtin_choose_expr(sizeof(v) <= sizeof(int), __builtin_clz(v), __builtin_choose_expr(sizeof(v) == sizeof(long int), __builtin_clzl(v), __builtin_clzll(v)))
#define     cdp_ctz(v)                __builtin_choose_expr(sizeof(v) <= sizeof(int), __builtin_ctz(v), __builtin_choose_expr(sizeof(v) == sizeof(long int), __builtin_ctzl(v), __builtin_ctzll(v)))

#define     cdp_bitsof(T)             (sizeof(T) << 3)
#define     cdp_bitson(v)             (cdp_bitsof(v) - cdp_clz(v))
#define     cdp_lengthof(a)           (sizeof(a)/sizeof(*a))

#define     cdp_dyn_size(Ts, Tm, l)   (sizeof(Ts) + ((l) * sizeof(Tm)))
#define     cdp_dyn_malloc(Ts, Tm, l) cdp_malloc(cdp_dyn_size(Ts, Tm, l))
#define     cdp_dyn_malloc0(Ts,Tm,l)  cdp_malloc0(cdp_dyn_size(Ts, Tm, l))

#define     cdp_is_pow_of_two(u)      (1 == cdp_popcount(u))
#define     cdp_max_pow_of_two(u)     (((typeof(u))1) << (cdp_bitsof(u) - 1))
#define     cdp_prev_pow_of_two(u)    (cdp_max_pow_of_two(u) >> cdp_clz(u))
#define     cdp_next_pow_of_two(u)    (cdp_is_pow_of_two(u)? u: (cdp_max_pow_of_two(u) >> (cdp_clz(u) - 1)))



/*
 * Value Checking
 */

#define     cdp_const_min(a, b)       ((a < b)? a: b)
#define     cdp_const_max(a, b)       ((a > b)? a: b)
#define     _cdp_min(_, _a, _b)       ({CDP_U(_,a, _a); CDP_U(_,b, _b);  cdp_const_min(CDP(_,a), CDP(_,b));})
#define     _cdp_max(_, _a, _b)       ({CDP_U(_,a, _a); CDP_U(_,b, _b);  cdp_const_max(CDP(_,a), CDP(_,b));})
#define     cdp_min(...)              _cdp_min(__COUNTER__, __VA_ARGS__)
#define     cdp_max(...)              _cdp_max(__COUNTER__, __VA_ARGS__)

#define     cdp_is_set(v, f)          (((v) & (f)) != 0)


/*
 * Function Utilities
 */

#define  CDP_BEFORE_MAIN_(...)        __attribute__((constructor(__VA_ARGS__)))
#define  CDP_AFTER_MAIN_(...)         __attribute__((destructor(__VA_ARGS__)))
  
#ifdef NDEBUG
  #define CDP_DEBUG()
#else
  #define CDP_DEBUG(code)   do{code;}while(0)
#endif


#endif
