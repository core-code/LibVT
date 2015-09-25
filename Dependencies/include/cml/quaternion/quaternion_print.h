/* -*- C++ -*- ------------------------------------------------------------
 
Copyright (c) 2007 Jesse Anders and Demian Nave http://cmldev.net/

The Configurable Math Library (CML) is distributed under the terms of the
Boost Software License, v1.0 (see cml/LICENSE for details).

 *-----------------------------------------------------------------------*/
/** @file
 *  @brief
 */

#ifndef quaternion_print_h
#define quaternion_print_h

#include <iostream>

namespace cml {

/* NOTE: Made 'plain' quaternion output the default (Jesse) */

/* #if !defined(CML_PLAIN_QUATERNION_OUTPUT) */
#if defined(CML_COMPLEX_QUATERNION_OUTPUT)

template<typename E, class AT, class CT> std::ostream&
operator<<(std::ostream& os, const cml::quaternion<E,AT,scalar_first,CT>& q)
{
    os << ((q[0] < 0)?" - ":"") << std::fabs(q[0]);
    os << ((q[1] < 0)?" - ":" + ") << std::fabs(q[1]) << "i";
    os << ((q[2] < 0)?" - ":" + ") << std::fabs(q[2]) << "j";
    os << ((q[3] < 0)?" - ":" + ") << std::fabs(q[3]) << "k";
    return os;
}

template<typename E, class AT, class CT> std::ostream&
operator<<(std::ostream& os, const cml::quaternion<E,AT,vector_first,CT>& q)
{
    os << ((q[0] < 0)?" - ":"") << std::fabs(q[0]) << "i";
    os << ((q[1] < 0)?" - ":" + ") << std::fabs(q[1]) << "j";
    os << ((q[2] < 0)?" - ":" + ") << std::fabs(q[2]) << "k";
    os << ((q[3] < 0)?" - ":" + ") << std::fabs(q[3]);
    return os;
}

#else

/** Output a quaternion to a std::ostream. */
template<typename E, class AT, class OT, typename CT> std::ostream&
operator<<(std::ostream& os, const cml::quaternion<E,AT,OT,CT>& q)
{
    os << "[";
    for (size_t i = 0; i < 4; ++i) {
        os << " " << q[i];
    }
    os << " ]";
    return os;
}

#endif

/** Output a quaternion expression to a std::ostream. */
template< class XprT > inline std::ostream&
operator<<(std::ostream& os, const et::QuaternionXpr<XprT>& q)
{
    typedef typename et::QuaternionXpr<XprT>::result_type quaternion_type;

    os << quaternion_type(q);
    /* XXX This temporary can be removed by templating the stream insertion
     * operators above.
     */

    return os;
}

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp
