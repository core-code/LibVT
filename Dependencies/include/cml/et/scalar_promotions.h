/* -*- C++ -*- ------------------------------------------------------------
 
Copyright (c) 2007 Jesse Anders and Demian Nave http://cmldev.net/

The Configurable Math Library (CML) is distributed under the terms of the
Boost Software License, v1.0 (see cml/LICENSE for details).

 *-----------------------------------------------------------------------*/
/** @file
 *  @brief
 */

#ifndef scalar_promotions_h
#define scalar_promotions_h

#include <cml/core/cml_meta.h>

namespace cml {
namespace et {

namespace detail {

/** @class IntPromote
 *  @brief Helper template to int-promote a type.
 */
template<class T> struct IntPromote
{
    /* Signed -> signed int, unsigned -> unsigned int: */
    typedef typename select_switch<T,
            unsigned char,                       unsigned int,
            unsigned short,                      unsigned int,
            signed char,                         int,
            char,                                int,
            short,                               int,
            T,                                   T
    >::result   result;
};

} // namespace detail

/** @class ScalarPromote
 *  @brief Template for compile-time type promotion via C promotion rules.
 */
template<class E1_in, class E2_in> struct ScalarPromote
{

    /* Integral-promote the types (if possible). */
    typedef typename detail::IntPromote<E1_in>::result  E1;
    typedef typename detail::IntPromote<E2_in>::result  E2;

    /* If sizeof(long) == sizeof(unsigned int), promote to unsigned long.
     * Otherwise, sizeof(long) > sizeof(int), so promote to long.
     */
    typedef typename select_if<sizeof(long) == sizeof(unsigned int),
            unsigned long,
            long
    >::result   uint_promotion;

    /* Do the selection on the promoted types: */
    typedef typename select_switch<
        type_pair<E1,E2>,

#if defined(CML_USE_LONG_DOUBLE)
        type_pair<long double,long double>,       long double,
        type_pair<long double,E2>,                long double,
        type_pair<E1,long double>,                long double,
#endif

        type_pair<double,double>,                 double,
        type_pair<double,E2>,                     double,
        type_pair<E1,double>,                     double,

        type_pair<float,float>,                   float,
        type_pair<float,E2>,                      float,
        type_pair<E1,float>,                      float,

        type_pair<E1,E2>,                         void

    >::result   float_filter;

    /* The promoted integral types really matter here: */
    typedef typename select_switch<
        type_pair<E1,E2>,

        type_pair<unsigned long,unsigned long>,   unsigned long,
        type_pair<unsigned long,E2>,              unsigned long,
        type_pair<E1,unsigned long>,              unsigned long,

        type_pair<long,long>,                     long,
        type_pair<long,unsigned int>,             uint_promotion,
        type_pair<unsigned int,long>,             uint_promotion,

        type_pair<long,E2>,                       long,
        type_pair<E1,long>,                       long,

        type_pair<unsigned int,unsigned int>,     unsigned int,
        type_pair<unsigned int,E2>,               unsigned int,
        type_pair<E1,unsigned int>,               unsigned int,

        type_pair<int,int>,                       int,
        type_pair<int,E2>,                        int,
        type_pair<E1,int>,                        int,

        type_pair<E1,E2>,                         void

    >::result   int_filter;

    /* Deduce the final type: */
    typedef typename select_if<
        same_type<float_filter,void>::is_true,
        int_filter, float_filter>::result         type;
};

} // namespace et
} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp
