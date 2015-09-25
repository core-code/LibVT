/* -*- C++ -*- ------------------------------------------------------------
 
Copyright (c) 2007 Jesse Anders and Demian Nave http://cmldev.net/

The Configurable Math Library (CML) is distributed under the terms of the
Boost Software License, v1.0 (see cml/LICENSE for details).

 *-----------------------------------------------------------------------*/
/** @file
 *  @brief
 */

#ifndef meta_switch_h
#define meta_switch_h

#include <cml/core/meta/common.h>
#include <cml/core/meta/if.h>

namespace cml {

struct NilCase {};      /* For terminating the case list. */
struct Default {};      /* For indicating the default result. */

/* The working parts of the meta-switch go into namespace meta: */
namespace meta {

/* "Interior" case statements: */
template<typename Case, typename Result, typename NextCase>
struct select_case
{
    template<typename Find> struct match {
        typedef typename select_if<
            same_type<Find,Case>::is_true,
            Result,
            typename NextCase::template match<Find>::result
        >::result result;
    };
};

/* Default case, returned when no match is found in a previous case: */
template<typename Result>
struct select_case<Default,Result,NilCase>
{
    template<typename Find> struct match {
        typedef Result result;
    };
};

/* The last case statement (if no match until now, the result is 'void'): */
template<typename Case, typename Result>
struct select_case<Case,Result,NilCase>
{
    template<typename Find> struct match {
        typedef typename select_if<
            same_type<Find,Case>::is_true,
            Result,
            void
        >::result result;
    };
};

} // namespace meta

/** Return the matched type (like a switch/case statement).
 *
 * This is a convenience wrapper to avoid having to explicitly type out
 * select_case for each case in the list of types to match against.
 */
template<typename Find
, typename T1,           typename R1
, typename T2 = NilCase, typename R2 = void
, typename T3 = NilCase, typename R3 = void
, typename T4 = NilCase, typename R4 = void
, typename T5 = NilCase, typename R5 = void
, typename T6 = NilCase, typename R6 = void
, typename T7 = NilCase, typename R7 = void
, typename T8 = NilCase, typename R8 = void
, typename T9 = NilCase, typename R9 = void
, typename T10 = NilCase, typename R10 = void
, typename T11 = NilCase, typename R11 = void
, typename T12 = NilCase, typename R12 = void
, typename T13 = NilCase, typename R13 = void
, typename T14 = NilCase, typename R14 = void
, typename T15 = NilCase, typename R15 = void
, typename T16 = NilCase, typename R16 = void
#if !defined(_MSC_VER)
, typename T17 = NilCase, typename R17 = void
, typename T18 = NilCase, typename R18 = void
, typename T19 = NilCase, typename R19 = void
, typename T20 = NilCase, typename R20 = void
, typename T21 = NilCase, typename R21 = void
, typename T22 = NilCase, typename R22 = void
, typename T23 = NilCase, typename R23 = void
, typename T24 = NilCase, typename R24 = void
, typename T25 = NilCase, typename R25 = void
, typename T26 = NilCase, typename R26 = void
, typename T27 = NilCase, typename R27 = void
, typename T28 = NilCase, typename R28 = void
, typename T29 = NilCase, typename R29 = void
, typename T30 = NilCase, typename R30 = void
, typename T31 = NilCase, typename R31 = void
, typename T32 = NilCase, typename R32 = void
, typename T33 = NilCase, typename R33 = void
, typename T34 = NilCase, typename R34 = void
, typename T35 = NilCase, typename R35 = void
, typename T36 = NilCase, typename R36 = void
, typename T37 = NilCase, typename R37 = void
, typename T38 = NilCase, typename R38 = void
, typename T39 = NilCase, typename R39 = void
, typename T40 = NilCase, typename R40 = void
#endif
> struct select_switch
{
    typedef typename
          meta::select_case< T1,R1
        , meta::select_case< T2,R2
        , meta::select_case< T3,R3
        , meta::select_case< T4,R4
        , meta::select_case< T5,R5
        , meta::select_case< T6,R6
        , meta::select_case< T7,R7
        , meta::select_case< T8,R8
        , meta::select_case< T9,R9
        , meta::select_case< T10,R10
        , meta::select_case< T11,R11
        , meta::select_case< T12,R12
        , meta::select_case< T13,R13
        , meta::select_case< T14,R14
        , meta::select_case< T15,R15
        , meta::select_case< T16,R16
#if !defined(_MSC_VER)
        , meta::select_case< T17,R17
        , meta::select_case< T18,R18
        , meta::select_case< T19,R19
        , meta::select_case< T20,R20
        , meta::select_case< T21,R21
        , meta::select_case< T22,R22
        , meta::select_case< T23,R23
        , meta::select_case< T24,R24
        , meta::select_case< T25,R25
        , meta::select_case< T26,R26
        , meta::select_case< T27,R27
        , meta::select_case< T28,R28
        , meta::select_case< T29,R29
        , meta::select_case< T30,R30
        , meta::select_case< T31,R31
        , meta::select_case< T32,R32
        , meta::select_case< T33,R33
        , meta::select_case< T34,R34
        , meta::select_case< T35,R35
        , meta::select_case< T36,R36
        , meta::select_case< T37,R37
        , meta::select_case< T38,R38
        , meta::select_case< T39,R39
        , meta::select_case< T40,R40
        , NilCase
        > > > > > > > > > >     /* 10 */
        > > > > > > > > > >     /* 10 */
        > > > >    /* 4 */
#else
        , NilCase
#endif
        > > > > > >     /* 6 */
        > > > > > > > > > >     /* 10 */
        ::template match<Find>::result result;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp
