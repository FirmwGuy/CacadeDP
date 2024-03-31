/*
 *  Copyright (c) 2021-2024 Victor M. Barrientos <firmw.guy@gmail.com>
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
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


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
//#include <iso646.h>


#define     CDPPASTIT(a,b)            a##b
#define     CDPPASTE(a,b)             CDPPASTIT(a, b)
#define     CDP(_, i)                 CDPPASTE(__##i, _)

#define     CDP_EXPECT(exp)           (__builtin_expect((long)(exp), true))
#define     CDP_RARELY(exp)           (__builtin_expect((long)(exp), false))



/*
 * Variable Initialization
 */
 
#define     CDP_T(v, x, ...)          __auto_type (x) __VA_ARGS__ = (v)
#define     CDP_U(_, x, v, ...)       CDP_T(v, CDP(_,x), ##__VA_ARGS__)
#define     CDP_P(T, p, a, ...)       T* (p) __VA_ARGS__ = (T*) (a)
#define     CDP_Q(_, p, T, a, ...)    CDP_P(T, CDP(_,p), a, ##__VA_ARGS__)

#define     CDP_AUTOFREE_NAME(n)      CDPPASTE(n, _AUTO)
#define     CDP_AUTOFREE_(f)          static inline void CDP_AUTOFREE_NAME(f)(void* p) {f(*(void**)p);}
#define     CDP_AT(a, p, f, ...)      CDP_T(a, p, __attribute__((cleanup(CDP_AUTOFREE_NAME(f)))), ##__VA_ARGS__)
#define     CDP_AU(_, a, p, f, ...)   CDP_V(_, a, p, __attribute__((cleanup(CDP_AUTOFREE_NAME(f)))), ##__VA_ARGS__)
#define     CDP_AP(T, p, f, a, ...)   CDP_P(T, p, a, __attribute__((cleanup(CDP_AUTOFREE_NAME(f)))), ##__VA_ARGS__)



/*
 * Memory Initialization
 */
 
#if  defined(CDP_MCU)  ||  (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
  #error    unsoported target platform!
#else
  static inline void*   cdp_malloc(size_t z)                   {void* p = malloc(z);       if CDP_RARELY(!p)  abort();  return p;}
  static inline void*   cdp_calloc(size_t n, size_t z)         {void* p = calloc(n, z);    if CDP_RARELY(!p)  abort();  return p;}
  static inline void*   cdp_realloc(void* p, size_t z)         {void* r = realloc(p, z);   if CDP_RARELY(!r)  abort();  return r;}
  
  static inline char*   cdp_strdup(const char* s)              {char* d = strdup(s);       if CDP_RARELY(!d)  abort();  return d;}
  static inline char*   cdp_strndup(const char* s, size_t z)   {char* d = strndup(s, z);   if CDP_RARELY(!d)  abort();  return d;}
  
  #define   cdp_alloca                alloca
  #define   cdp_free                  free
#endif                               
#define     cdp_malloc0(z)            cdp_calloc(1, z)
                                     
                                     
CDP_AUTOFREE_(cdp_free)              
                                     
                                     
#define     cdp_new(T, ...)           cdp_malloc(sizeof(T) __VA_ARGS__)
#define     cdp_new0(T, ...)          cdp_malloc0(sizeof(T) __VA_ARGS__)
#define     CDP_NEW0(T, p, ...)       CDP_P(T, p, cdp_new0(T, ##__VA_ARGS__))
#define     CDP_NEW(T, p, ...)        CDP_P(T, p, cdp_new(T, ##__VA_ARGS__))
#define     CDP_FR(T, p, ...)         T* (p) __attribute__((cleanup(CDP_AUTOFREE_NAME(cdp_free)))) __VA_ARGS__
#define     CDP_FREE(p)               do{ cdp_free(p); (p) = NULL; }while (0)


static inline void  cdp_cpy_or_0(void* d, void* p, size_t z)  {assert(d);  if (p) memcpy(d, p, z); else memset(d, 0, z);}
static inline void* cdp_clone(void* p, size_t z)              {void* c; if (!p || !z) c = NULL; else {c = cdp_malloc(z); memcpy(c, p, z);}  return c;}

#define     CDP_CLONE(T, d, p)        T* (d) = cdp_clone(p, sizeof(T))
#define     CDP_0(p)                  memset(p, 0, sizeof(*(p)))
#define     _CDP_SWAP(_, a, b)        do{ CDP_U(_,c, a); (a) = (b); (b) = CDP(_,c); }while(0)
#define     CDP_SWAP(...)             _CDP_SWAP(__COUNTER__, __VA_ARGS__)



/*
 * Pointer Utilities
 */
 
#define     _cdp_align_to(_, u, _a)   ({CDP_U(_,a, _a);  assert(0 < CDP(_,a));  ((u) + (CDP(_,a) - 1)) & ~(CDP(_,a) - 1);})
#define     cdp_align_to(...)         _cdp_align_to(__COUNTER__, __VA_ARGS__)
#define     cdp_align_max(u)          cdp_align_to(u, __BIGGEST_ALIGNMENT__)
#define     cdp_aligned(t)            cdp_align_to(sizeof(t), __alignof__(t))
#define     CDP_ALIGN_TO(u, a)        ((u) = cdp_align_to(u, a))

#define     cdp_ptr_align_to(p, a)    ((void*)cdp_align_to((uintptr_t)(p), a))
#define     cdp_ptr_aligned(p)        ((void*)cdp_aligned((uintptr_t)(p)))
#define     cdp_ptr_off(p, off)       ((void*)(((void*)(p)) + (off)))
#define     CDP_PTR_OFF(p, off)       ((p) = cdp_ptr_off(p, off))
#define     cdp_ptr_dif(p1, p2)       (((void*)(p1)) - ((void*)(p2)))
#define     cdp_ptr_idx(p, o, z)      (cdp_ptr_dif(o, p)/(z))
#define     cdp_ptr_adr(p, i, z)      cdp_ptr_off(p, (i)*(z))
#define     cdp_ptr_has_val(p)        ((p) && *(p))


#define     cdp_popcount(v)           __builtin_choose_expr(sizeof(v) <= sizeof(int), __builtin_popcount(v), __builtin_choose_expr(sizeof(v) == sizeof(long int), __builtin_popcountl(v), __builtin_popcountll(v)))
#define     cdp_clz(v)                __builtin_choose_expr(sizeof(v) <= sizeof(int), __builtin_clz(v), __builtin_choose_expr(sizeof(v) == sizeof(long int), __builtin_clzl(v), __builtin_clzll(v)))
#define     cdp_ctz(v)                __builtin_choose_expr(sizeof(v) <= sizeof(int), __builtin_ctz(v), __builtin_choose_expr(sizeof(v) == sizeof(long int), __builtin_ctzl(v), __builtin_ctzll(v)))

#define     cdp_bitsof(T)             (sizeof(T) << 3)
#define     cdp_bitson(v)             (cdp_bitsof(v) - cdp_clz(v))
#define     cdp_lengthof(a)           (sizeof(a)/sizeof(*a))

#define     cdp_is_pow_of_two(u)      (1 == cdp_popcount(u))
#define     cdp_max_pow_of_two(u)     (((typeof(u))1) << (cdp_bitsof(u) - 1))
#define     cdp_prev_pow_of_two(u)    (cdp_max_pow_of_two(u) >> cdp_clz(u))
#define     cdp_next_pow_of_two(u)    (cdp_is_pow_of_two(u)? u: (cdp_max_pow_of_two(u) >> (cdp_clz(u) - 1)))



/*
 * Bounds Checking
 */
 
#define     cdp_const_min(a, b)       ((a < b)? a: b)
#define     cdp_const_max(a, b)       ((a > b)? a: b)
#define     _cdp_min(_, _a, _b)       ({CDP_U(_,a, _a); CDP_U(_,b, _b);  cdp_const_min(CDP(_,a), CDP(_,b));})
#define     _cdp_max(_, _a, _b)       ({CDP_U(_,a, _a); CDP_U(_,b, _b);  cdp_const_max(CDP(_,a), CDP(_,b));})
#define     cdp_min(...)              _cdp_min(__COUNTER__, __VA_ARGS__)
#define     cdp_max(...)              _cdp_max(__COUNTER__, __VA_ARGS__)

#define     cdp_in_r(x, l, u)         ((x) >= (l)  &&  (x) <= (u))
#define     cdp_insd(x, l, u)         ((x) >  (l)  &&  (x)  < (u))

#define     CDP_DEFAULT(x, d)         ({if (!(x)) (x) = (d);  (x);})
#define     _CDP_TRUNCATE(_, x, _u)   (__builtin_constant_p(_u)?  (((x) > (_u))? ((x) = (_u)): (x))  :  ({CDP_U(_,u, _u);  if ((x) > CDP(_,u)) (x) = CDP(_,u);  (x);}))
#define     _CDP_PROLONG(_, x, _l)    (__builtin_constant_p(_l)?  (((x) < (_l))? ((x) = (_l)): (x))  :  ({CDP_U(_,l, _l);  if ((x) < CDP(_,l)) (x) = CDP(_,l);  (x);}))
#define     CDP_TRUNCATE(...)         _CDP_TRUNCATE(__COUNTER__, __VA_ARGS__)
#define     CDP_PROLONG(...)          _CDP_PROLONG(__COUNTER__, __VA_ARGS__)


#define     CDP_IF_DO(e, x, ...)      do{ if (e) {__VA_ARGS__; x;} }while (0)
#define     CDP_QT(e, ...)            CDP_IF_DO((e), return, ##__VA_ARGS__)
#define     CDP_CK(e, ...)            CDP_IF_DO(!(e), return 0, ##__VA_ARGS__)
#define     CDP_GO(e, ...)            CDP_IF_DO((e), return true, ##__VA_ARGS__)
#define     CDP_ER(e, ...)            CDP_IF_DO((e), goto CDP_ERROR, ##__VA_ARGS__)
#define     CDP_BR(e, ...)            CDP_IF_DO((e), goto CDP_BREAK, ##__VA_ARGS__)



/*
 * Binary Search
 */
 
typedef     int                       (*cdpCmp)          (const void*, const void*);
typedef     void                      (*cdpDel)          (void*);
typedef     intptr_t                  (*cdpCallBack)     (void*, void*);


#define     CDP_CMP(a, b, X)          (((a)->X < (b)->X)? -1: ((a)->X - (b)->X))
#define     CDP_CMPi(a, b, I)         ((a)->I - (b)->I)
#define     CDP_FUNC_CMP_(f, T, X)    int f(const void* restrict a, const void* restrict b) {assert(a && b); return CDP_CMP((T*)a, (T*)b, X);}
#define     CDP_FUNC_CMPi_(f, T, I)   int f(const void* restrict a, const void* restrict b) {assert(a && b); return CDP_CMPi((T*)a, (T*)b, I);}


#define     _cdp_search_w_idx(_, _k, _b, _n, _z, i, f, ...)                    \
({                                                                             \
    __label__ CDP(_,END);                                                      \
    CDP_U(_,k, _k);  CDP_U(_,b, _b);  CDP_U(_,n, _n);  CDP_U(_,z, _z);         \
    assert(CDP(_,k) && CDP(_,b) && CDP(_,z));                                  \
    void* CDP(_,p);                                                            \
                                                                               \
    if CDP_EX(CDP(_,n))                                                        \
    {                                                                          \
        size_t CDP(_,imax) = CDP(_,n) - 1,  CDP(_,imin) = 0;                   \
        do {                                                                   \
            (i) = (CDP(_,imin) + CDP(_,imax)) >> 1;                            \
                                                                               \
            CDP(_,p) = cdp_ptr_off(CDP(_,b), (i) * CDP(_,z));                  \
                                                                               \
            int CDP(_,cmpr) = f(CDP(_,k), CDP(_,p), ##__VA_ARGS__);            \
                                                                               \
            if (0 > CDP(_,cmpr))                                               \
            {                                                                  \
                if (0 == (i))   break;                                         \
                CDP(_,imax) = (i) - 1;                                         \
            }                                                                  \
            else                                                               \
            if (0 < CDP(_,cmpr))                                               \
                CDP(_,imin) = ++(i);                                           \
            else                                                               \
                goto CDP(_,END);                                               \
        }                                                                      \
        while (CDP(_,imax) >= CDP(_,imin));                                    \
    }                                                                          \
    else                                                                       \
        (i) = 0;                                                               \
    CDP(_,p) = NULL;                                                           \
                                                                               \
CDP(_,END):                                                                    \
    CDP(_,p);                                                                  \
})
#define     cdp_search_w_idx(...)                  _cdp_search_w_idx(__COUNTER__, __VA_ARGS__)

#define     _cdp_search(_, k, b, n, z, f, ...)     ({size_t CDP(_,i); cdp_search_w_idx(k, b, n, z, CDP(_,i), f, ##__VA_ARGS__);})
#define     cdp_search(...)                        _cdp_search(__COUNTER__, __VA_ARGS__)



/*
 * Nested Looping
 */
 
#define     CDP_FOR_INI_(i, e, ...)   for (i;; (__VA_ARGS__)) { if (e)
#define     CDP_FOR_(e, ...)          CDP_FOR_INI_(,e,##__VA_ARGS__)
#define     CDP_WHILE_(e)             CDP_FOR_(e)
#define     CDP_ELSE                  else {
#define     _CDP_ELSE_ROF             break;}}
#define     __CDP_ELSE_ROF            _CDP_ELSE_ROF}
#define     _CDP_ROF                  CDP_ELSE  _CDP_ELSE_ROF
#define     __CDP_ROF                 _CDP_ROF}



/*
 * Generic Vector Array
 */
 
#define     _cdp_vect_append_n(_, _v, _z, m, l, p, _n)                         \
({                                                                             \
    void* CDP(_,v) = _v;   CDP_U(_,z, _z);    CDP_U(_,n, _n);                  \
    assert(CDP(_,v) && CDP(_,z) && CDP(_,n)  &&  (m) >= ((l) + CDP(_,n)));     \
                                                                               \
    void* CDP(_,o) = cdp_ptr_adr(CDP(_,v), (l), CDP(_,z));                     \
    cdp_cpy_or_0(CDP(_,o), p, CDP(_,n) * CDP(_,z));                            \
    (l) += CDP(_,n);                                                           \
                                                                               \
    CDP(_,o);                                                                  \
})
#define     cdp_vect_append_n(...)             _cdp_vect_append_n(__COUNTER__, __VA_ARGS__)
#define     cdp_vect_append(v, z, m, l, p)     cdp_vect_append_n(v, z, m, l, p, 1)

#define     _cdp_vect_push_n(_, _v, _z, m, l, _p, _n)                          \
({                                                                             \
    void* CDP(_,v) = _v;   CDP_U(_,z, _z);                                     \
    void* CDP(_,p) = _p;   CDP_U(_,n, _n);                                     \
    assert(CDP(_,v) && CDP(_,z) && CDP(_,n)  &&  (m) >= ((l) + CDP(_,n)));     \
                                                                               \
    size_t CDP(_,addz) = CDP(_,n) * CDP(_,z);                                  \
    void* CDP(_,o) = CDP(_,v);                                                 \
                                                                               \
    if (l)  memmove(  cdp_ptr_off(CDP(_,v), CDP(_,addz)),                      \
                      CDP(_,v),                                                \
                      (l) * CDP(_,z)  );                                       \
                                                                               \
    cdp_cpy_or_0(CDP(_,o), CDP(_,p), CDP(_,addz));                             \
    (l) += CDP(_,n);                                                           \
                                                                               \
    CDP(_,o);                                                                  \
})
#define     cdp_vect_push_n(...)              _cdp_vect_push_n(__COUNTER__, __VA_ARGS__)
#define     cdp_vect_push(v, z, m, l, p)      cdp_vect_push_n(v, z, m, l, p, 1)

#define     _cdp_vect_pop(_, _v, _z, m, l, _o)                                 \
({                                                                             \
    void* CDP(_,v) = _v;  CDP_U(_,z, _z);                                      \
    void* CDP(_,o) = _o;                                                       \
    assert(CDP(_,v) && CDP(_,z)  &&  (m) >= (l));                              \
                                                                               \
    if (l)                                                                     \
    {                                                                          \
        (l)--;                                                                 \
                                                                               \
        if (CDP(_,o))   memcpy(CDP(_,o), CDP(_,v), CDP(_,z));                  \
                                                                               \
        if (l)          memmove(  CDP(_,v),                                    \
                                  cdp_ptr_off(CDP(_,v), CDP(_,z)),             \
                                  (l) * CDP(_,z)  );                           \
    }                                                                          \
    else                                                                       \
        CDP(_,o) = NULL;                                                       \
                                                                               \
    CDP(_,o);                                                                  \
})
#define     cdp_vect_pop(...)     _cdp_vect_pop(__COUNTER__, __VA_ARGS__)

#define     _cdp_vect_pop_last(_, _v, _z, m, l, _o)                            \
({                                                                             \
    void* CDP(_,v) = _v;  CDP_U(_,z, _z);   void* CDP(_,o) = _o;               \
    assert(CDP(_,v) && CDP(_,z)  &&  (m) >= (l));                              \
                                                                               \
    if (l)                                                                     \
    {                                                                          \
        (l)--;                                                                 \
                                                                               \
        if (CDP(_,o))   memcpy(CDP(_,o), CDP(_,v), CDP(_,z));                  \
    }                                                                          \
    else                                                                       \
        CDP(_,o) = NULL;                                                       \
                                                                               \
    CDP(_,o);                                                                  \
})
#define     cdp_vect_pop_last(...)    _cdp_vect_pop_last(__COUNTER__, __VA_ARGS__)

#define     _cdp_vect_insert_n(_, _v, _z, _m, l, _i, _p, _n)                   \
({                                                                             \
    CDP_U(_,z, _z);   CDP_U(_,m, _m);   CDP_U(_,i, _i);   CDP_U(_,n, _n);      \
    void* CDP(_,v) = _v;  void* CDP(_,p) = _p;  void* CDP(_,o);                \
    assert(CDP(_,v) && CDP(_,z) && CDP(_,n)                                    \
       &&  CDP(_,m) >= ((l) + CDP(_,n)));                                      \
                                                                               \
    if ((l) > CDP(_,i))                                                        \
    {                                                                          \
        size_t CDP(_,tocp) = (l) - CDP(_,i);                                   \
        size_t CDP(_,addz) = CDP(_,n) * CDP(_,z);                              \
        CDP(_,o) = cdp_ptr_adr(CDP(_,v), CDP(_,i), CDP(_,z));                  \
                                                                               \
        memmove(  cdp_ptr_off(CDP(_,o), CDP(_,addz)),                          \
                  CDP(_,o),                                                    \
                  CDP(_,tocp) * CDP(_,z)  );                                   \
                                                                               \
        cdp_cpy_or_0(CDP(_,o), CDP(_,p), CDP(_,addz));                         \
        (l) += CDP(_,n);                                                       \
    }                                                                          \
    else if (l == CDP(_,i))                                                    \
        CDP(_,o) = cdp_vect_append_n( CDP(_,v), CDP(_,z),                      \
                                      CDP(_,m), l,                             \
                                      CDP(_,p), CDP(_,n)  );                   \
    else                                                                       \
        CDP(_,o) = NULL;                                                       \
                                                                               \
    CDP(_,o);                                                                  \
})
#define     cdp_vect_insert_n(...)              _cdp_vect_insert_n(__COUNTER__, __VA_ARGS__)
#define     cdp_vect_insert(v, z, m, l, i, p)   cdp_vect_insert_n(v, z, m, l, i, p, 1)

#define     _cdp_vect_remove(_, _v, _z, m, l, _i, _o)                          \
({                                                                             \
    CDP_U(_,z, _z);   CDP_U(_,i, _i);                                          \
    void* CDP(_,v) = _v;  void* CDP(_,o) = _o;                                 \
    assert(CDP(_,v) && CDP(_,z)  &&  (m) >= (l));                              \
                                                                               \
    if ((l) > CDP(_,i))                                                        \
    {                                                                          \
        void* CDP(_,r) = cdp_ptr_adr(CDP(_,v), CDP(_,i), CDP(_,z));            \
        if (CDP(_,o))   memcpy(CDP(_,o), CDP(_,r), CDP(_,z));                  \
                                                                               \
        (l)--;                                                                 \
        if (l)                                                                 \
        {                                                                      \
            size_t CDP(_,tocp) = (l) - CDP(_,i);                               \
            if (CDP(_,tocp))  memmove(  CDP(_,r),                              \
                                        cdp_ptr_off(CDP(_,r), CDP(_,z)),       \
                                        CDP(_,tocp) * CDP(_,z)  );             \
        }                                                                      \
    }                                                                          \
    else                                                                       \
        CDP(_,o) = NULL;                                                       \
                                                                               \
    CDP(_,o);                                                                  \
})
#define     cdp_vect_remove(...)               _cdp_vect_remove(__COUNTER__, __VA_ARGS__)

#define     _CDP_VECT_FOR_EACH__(_, v, _z, m, _l, p, ...)  {__VA_ARGS__ (p) = (v); CDP_U(_,z, _z); CDP_U(_,l, _l);  assert((p) && CDP(_,z) && (m)>=CDP(_,l));  size_t CDP(_,n)=0; CDP_FOR_(CDP(_,n) < CDP(_,l), CDP(_,n)++, CDP_PTR_OFF(p, CDP(_,z)))
#define     CDP_VECT_FOR_EACH__(...)                       _CDP_VECT_FOR_EACH__(__COUNTER__, __VA_ARGS__)

#define     _cdp_vect_traverse(_, v, z, m, l, f, ...)      ({void* CDP(_,p); CDP_VECT_FOR_EACH__(v, z, m, l, CDP(_,p)) {if (!f(CDP(_,p), ##__VA_ARGS__)) break;} CDP_ELSE {CDP(_,p) = NULL;}__CDP_ELSE_ROF  CDP(_,p);})
#define     cdp_vect_traverse(...)                         _cdp_vect_traverse(__COUNTER__, __VA_ARGS__)

#define     _cdp_vect_lfind(_, v, z, m, l, _k, f, ...)     ({CDP_U(_,k, _k);  assert(CDP(_,k));  void* CDP(_,p); CDP_VECT_FOR_EACH__(v, z, m, l, CDP(_,p)) {if (!f(CDP(_,k), CDP(_,p), ##__VA_ARGS__)) break;} CDP_ELSE {CDP(_,p) = NULL;}__CDP_ELSE_ROF  CDP(_,p);})
#define     cdp_vect_lfind(...)                            _cdp_vect_lfind(__COUNTER__, __VA_ARGS__)

#define     _cdp_vect_search_from_idx(_, _v, _z, m, _l, k, i, f, ...)          \
({                                                                             \
    void* CDP(_,v) = _v;  CDP_U(_,z, _z);   CDP_U(_,l, _l);                    \
    assert(CDP(_,v) && CDP(_,z));                                              \
    void* CDP(_,p);                                                            \
                                                                               \
    if (CDP(_,l) > (i))                                                        \
    {                                                                          \
        CDP(_,p) = cdp_search_w_idx(  k, CDP(_,v), (i), CDP(_,z),              \
                                      (i),                                     \
                                      f, ##__VA_ARGS__  );                     \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        (i) = 0;                                                               \
        CDP(_,p) = NULL;                                                       \
    }                                                                          \
                                                                               \
    CDP(_,p);                                                                  \
})
#define     cdp_vect_search_from_idx(...)     _cdp_vect_search_from_idx(__COUNTER__, __VA_ARGS__)

#define     cdp_vect_search_w_idx(v, z, m, l, k, i, f, ...)   cdp_search_w_idx(k, v, l, z, i, f, ##__VA_ARGS__)
#define     cdp_vect_search(v, z, m, l, k, f, ...)            cdp_search(k, v, l, z, f, ##__VA_ARGS__)

#define     _cdp_vect_sorted_insert(_, _v, _z, _m, l, _k, f, ...)              \
({                                                                             \
    void* CDP(_,v) =_v;  CDP_U(_,z, _z);  CDP_U(_,m, _m);  void* CDP(_,k) =_k; \
    assert(CDP(_,v) && CDP(_,z) && CDP(_,k)  &&  CDP(_,m) >= (l));             \
    size_t CDP(_,i);                                                           \
                                                                               \
    void* CDP(_,p) = cdp_vect_search_w_idx( CDP(_,v), CDP(_,z),                \
                                            CDP(_,m), l,                       \
                                            CDP(_,k), CDP(_,i),                \
                                            f, ##__VA_ARGS__  );               \
    if (CDP(_,p))                                                              \
        memcpy(CDP(_,p), CDP(_,k), CDP(_,z));                                  \
    else                                                                       \
        CDP(_,p) = cdp_vect_insert( CDP(_,v), CDP(_,z),                        \
                                    CDP(_,m), l,                               \
                                    CDP(_,i), CDP(_,k)  );                     \
    CDP(_,p);                                                                  \
})
#define     cdp_vect_sorted_insert(...)     _cdp_vect_sorted_insert(__COUNTER__, __VA_ARGS__)



/*
 * Generic Linked List
 */
 
typedef struct {
    void* next;
} cdpList;


#define     CDPTYPE(st)           CDPPASTE(cdp, st)
#define     CDPITEM(st)           CDPPASTE(CDPTYPE(st), _Item)
#define     CDP_ITEM_TYPE(st)     typedef struct CDPPASTE(_, CDPITEM(st)) {CDPTYPE(st); void* data;} CDPITEM(st)
#define     cdpITEM(st, p)        ((CDPITEM(st)*)(p))

CDP_ITEM_TYPE(List);

#define     cdpLIST(p)                 ((cdpList*)(p))


#define     _CDP_LIST_APPEND(_, l, _n)              ({CDP_Q(_,n, cdpList, _n);  assert(CDP(_,n));  CDP_Q(_,m, cdpList, l);  if (CDP(_,m)) {while (CDP(_,m)->next) {CDP(_,m) = CDP(_,m)->next;} CDP(_,m)->next = CDP(_,n);} else (l) = (void*)CDP(_,n);  (void*)CDP(_,n);})
#define     CDP_LIST_APPEND(...)                    _CDP_LIST_APPEND(__COUNTER__, __VA_ARGS__)
                                                 
#define     _CDP_LIST_APPEND_ITEM(_, l, p)          ({CDP_NEW(CDPITEM(List), CDP(_,n)); CDP(_,n)->data = (p);  CDP_LIST_APPEND(l, CDP(_,n));})
#define     CDP_LIST_APPEND_ITEM(...)               _CDP_LIST_APPEND_ITEM(__COUNTER__, __VA_ARGS__))
                                                 
#define     _CDP_LIST_PUSH(_, l, _n)                ({CDP_Q(_,n, cdpList, _n);  assert(CDP(_,n));  CDP(_,n)->next = (l); (l) = (void*)CDP(_,n);  (void*)CDP(_,n);})
#define     CDP_LIST_PUSH(...)                      _CDP_LIST_PUSH(__COUNTER__, __VA_ARGS__)
                                                 
#define     _CDP_LIST_PUSH_ITEM(_, l, p)            ({CDP_NEW(CDPITEM(List), CDP(_,n)); CDP(_,n)->data = (p);  CDP_LIST_PUSH(l, CDP(_,n));})
#define     CDP_LIST_PUSH_ITEM(...)                 _CDP_LIST_PUSH_ITEM(__COUNTER__, __VA_ARGS__)
                                                 
#define     _CDP_LIST_POP(_, l)                     ({CDP_Q(_,n, cdpList, l);  if (CDP(_,n)) (l) = (void*)CDP(_,n)->next;  (void*)CDP(_,n);})
#define     CDP_LIST_POP(...)                       _CDP_LIST_POP(__COUNTER__, __VA_ARGS__)
                                                 
#define     _CDP_LIST_POP_LAST(_, l)                ({CDP_Q(_,n, cdpList, l);  if (CDP(_,n)) {cdpList* CDP(_,p) = NULL; while (CDP(_,n)->next) {CDP(_,p) = CDP(_,n); CDP(_,n) = CDP(_,n)->next;} if (CDP(_,p)) CDP(_,p)->next = NULL; else (l) = NULL;}  (void*)CDP(_,n);})
#define     CDP_LIST_POP_LAST(...)                  _CDP_LIST_POP_LAST(__COUNTER__, __VA_ARGS__)
                                                 
#define     CDP_LIST_FOR_EACH_(l, n, ...)           for (__VA_ARGS__ (n) = (void*)(l);;  (n) = (n)->next) { if (n)

#define     _CDP_LIST_FOR_EACH_ITEM_(_, _l, p,...)  for (CDP_Q(_,n, CDPITEM(List), _l); ;  CDP(_,n) = CDP(_,n)->next) {__VA_ARGS__ (p); if (CDP(_,n) && ((p) = CDP(_,n)->data))
#define     CDP_LIST_FOR_EACH_ITEM_(...)            _CDP_LIST_FOR_EACH_ITEM_(__COUNTER__, __VA_ARGS__)

#define     _cdp_list_traverse(_, _l, f, ...)       ({CDP_Q(_,n, cdpList, _l);  while (CDP(_,n)  &&  (f)((void*)CDP(_,n), ##__VA_ARGS__)) {CDP(_,n) = CDP(_,n)->next;}  (void*)CDP(_,n);})
#define     cdp_list_traverse(...)                  _cdp_list_traverse(__COUNTER__, __VA_ARGS__)
                                                   
#define     _cdp_list_traverse_item(_, l, f, ...)   ({void* CDP(_,i) = NULL;  CDP_LIST_FOR_EACH_ITEM_(l, CDP(_,p), cdpList*) {if CDP_RARELY(!(f)((void*)CDP(_,p), ##__VA_ARGS__)) {CDP(_,i) = CDP(_,p); break;}}_CDP_ROF  CDP(_,i);})
#define     cdp_list_traverse_item(...)             _cdp_list_traverse_item(__COUNTER__, __VA_ARGS__)
                                                   
#define     _cdp_list_lfind(_, _l, _k, f, ...)      ({CDP_Q(_,n, cdpList, _l); CDP_U(_,k, _k);  assert(CDP(_,k));  while CDP_EX(CDP(_,n)  &&  (f)(CDP(_,k), (void*)CDP(_,n), ##__VA_ARGS__))  {CDP(_,n) = CDP(_,n)->next;}  (void*)CDP(_,n);})
#define     cdp_list_lfind(...)                     _cdp_list_lfind(__COUNTER__, __VA_ARGS__)
                                                   
#define     _cdp_list_lfind_item(_, l, _k, f, ...)  ({CDP_U(_,k, _k);  assert(CDP(_,k));  void* CDP(_,i) = NULL;  CDP_LIST_FOR_EACH_ITEM_(l, CDP(_,p), cdpList*) {if CDP_RARELY(!(f)(CDP(_,k), (void*)CDP(_,p), ##__VA_ARGS__)) {CDP(_,i) = CDP(_,p); break;}}_CDP_ROF  CDP(_,i);})
#define     cdp_list_lfind_item(...)                _cdp_list_lfind_item(__COUNTER__, __VA_ARGS__)
                                                   
#define     CDP_LIST_INVERT(l)                      ({cdpList* CDP(_,n), *CDP(_,p) = NULL; while (l) {CDP(_,n) = (l)->next; (l)->next = CDP(_,p); CDP(_,p) = cdpLIST(l); (l) = (void*)CDP(_,n);}  (void*)CDP(_,p);})

static inline void* cdp_list_adrof(void* l, size_t i)         {size_t j = 0; CDP_LIST_FOR_EACH_(l, p, cdpList*) {if (j == i) return p; j++;}_CDP_ROF return NULL;}
static inline void  cdp_list_insert_after(void* n, void* p)   {assert(n && p);  cdpLIST(p)->next = cdpLIST(n)->next; cdpLIST(n)->next = p;}
static inline void* cdp_list_remove_after(void* n)            {assert(n && cdpLIST(n)->next);  cdpList* m = cdpLIST(n)->next; cdpLIST(n)->next = m->next; return m;}

#define     _CDP_LIST_INSERT(_, l, _i, _p)                                     \
({                                                                             \
    CDP_U(_,i, _i);   size_t CDP(_,j) = 0;                                     \
    cdpList* CDP(_,n), *CDP(_,r) = NULL, *CDP(_,p) = cdpLIST(_p);              \
    assert(CDP(_,p));                                                          \
                                                                               \
    CDP_LIST_FOR_EACH_(l, CDP(_,n))                                            \
    {                                                                          \
        if (CDP(_,j) == CDP(_,i))                                              \
        {                                                                      \
            if (CDP(_,r))   cdp_list_insert_after(CDP(_,r), CDP(_,p));         \
            else            CDP_LIST_PUSH(l, CDP(_,p));                        \
            break;                                                             \
        }                                                                      \
        CDP(_,j)++;                                                            \
        CDP(_,r) = CDP(_,n);                                                   \
    }                                                                          \
    CDP_ELSE                                                                   \
    {                                                                          \
        if (0 == CDP(_,j))  CDP_LIST_PUSH(l, CDP(_,p));                        \
        else                CDP(_,p) = NULL;                                   \
    }_CDP_ELSE_ROF                                                             \
                                                                               \
    (void*)CDP(_,p);                                                           \
})
#define     CDP_LIST_INSERT(...)      _CDP_LIST_INSERT(__COUNTER__, __VA_ARGS__)

#define     _CDP_LIST_REMOVE(_, l, _i)                                         \
({                                                                             \
    CDP_U(_,i, _i);   size_t CDP(_,j) = 0;                                     \
    cdpList* CDP(_,n), *CDP(_,r) = NULL;                                       \
                                                                               \
    CDP_LIST_FOR_EACH_(l, CDP(_,n))                                            \
    {                                                                          \
        if (CDP(_,j) == CDP(_,i))                                              \
        {                                                                      \
            if (CDP(_,r))   cdp_list_remove_after(CDP(_,r));                   \
            else            CDP(_,n) = CDP_LIST_POP(l);                        \
            break;                                                             \
        }                                                                      \
        CDP(_,j)++;                                                            \
        CDP(_,r) = CDP(_,n);                                                   \
    }_CDP_ROF                                                                  \
                                                                               \
    (void*)CDP(_,n);                                                           \
})
#define     CDP_LIST_REMOVE(...)               _CDP_LIST_REMOVE(__COUNTER__, __VA_ARGS__)

#define     _CDP_LIST_SORT(_, l, f, ...)                                       \
do{                                                                            \
    if (!(l)) break;                                                           \
                                                                               \
    cdpList* CDP(_,p) = cdpLIST(l),      *CDP(_,q);                            \
    cdpList* CDP(_,n) = CDP(_,p)->next,  *CDP(_,m);                            \
                                                                               \
    while (CDP(_,n))                                                           \
    {                                                                          \
        if (0 > f((void*)CDP(_,n), (void*)CDP(_,p), ##__VA_ARGS__))            \
        {                                                                      \
            CDP(_,p)->next = CDP(_,n)->next;                                   \
                                                                               \
            for (   CDP(_,m) = cdpLIST(l),    CDP(_,q) = NULL;                 \
                    CDP(_,m) != CDP(_,p);                                      \
                    CDP(_,m) = CDP(_,m)->next )                                \
            {                                                                  \
                if (0 > f((void*)CDP(_,n), (void*)CDP(_,m), ##__VA_ARGS__))    \
                    break;                                                     \
                CDP(_,q) = CDP(_,m);                                           \
            }                                                                  \
                                                                               \
            if (CDP(_,q))  CDP(_,q)->next = CDP(_,n);                          \
            else            (l) = (void*)CDP(_,n);                             \
            CDP(_,n)->next = CDP(_,m);                                         \
                                                                               \
            CDP(_,n) = CDP(_,p)->next;                                         \
        }                                                                      \
        else                                                                   \
        {                                                                      \
            CDP(_,p) = CDP(_,n);                                               \
            CDP(_,n) = CDP(_,n)->next;                                         \
        }                                                                      \
    }                                                                          \
}while(0)
#define     CDP_LIST_SORT(...)             _CDP_LIST_SORT(__COUNTER__, __VA_ARGS__)

#define     _CDP_LIST_SORTED_INSERT(_, l, _k, f, ...)                          \
({                                                                             \
    CDP_U(_, k, _k);                                                           \
    assert(CDP(_,k));                                                          \
    cdpList* CDP(_,n) = cdpLIST(l), *CDP(_,p) = NULL;                          \
                                                                               \
    while (CDP(_,n))                                                           \
    {                                                                          \
        if (0 > f(CDP(_,k), (void*)CDP(_,n), ##__VA_ARGS__))                   \
            break;                                                             \
                                                                               \
        CDP(_,p) = CDP(_,n);                                                   \
        CDP(_,n) = CDP(_,n)->next;                                             \
    }                                                                          \
                                                                               \
    if (CDP(_,p))  CDP(_,p)->next = CDP(_,k);                                  \
    else            (l) = (void*)CDP(_,k);                                     \
                                                                               \
    cdpLIST(CDP(_,k))->next = CDP(_,n);                                        \
                                                                               \
    CDP(_,k);                                                                  \
})
#define     CDP_LIST_SORTED_INSERT(...)                _CDP_LIST_SORTED_INSERT(__COUNTER__, __VA_ARGS__)

#define     _cdp_list_del_all(_, _l, del, ...)         do{ CDP_Q(_,l, cdpList, _l);  void* CDP(_,n);  while(CDP(_,l)) {CDP(_,n) = CDP(_,l); CDP(_,l) = CDP(_,l)->next; (del)(CDP(_,n), ##__VA_ARGS__);} }while(0)
#define     cdp_list_del_all(...)                      _cdp_list_del_all(__COUNTER__, __VA_ARGS__)

#define     _cdp_list_del_all_item(_, _l, del, ...)    do{ CDP_Q(_,l, CDPITEM(List), _l), *CDP(_,n);  while(CDP(_,l)) {CDP(_,n) = CDP(_,l); CDP(_,l) = CDP(_,l)->next; (del)(CDP(_,n)->data, ##__VA_ARGS__); cdp_free(CDP(_,n));} }while(0)
#define     cdp_list_del_all_item(...)                 _cdp_list_del_all_item(__COUNTER__, __VA_ARGS__)

#define     CDP_LIST_DEL_ALL(l, del, ...)              do{ cdp_list_del_all(l, del, ##__VA_ARGS__); (l) = NULL; }while(0)


#endif
