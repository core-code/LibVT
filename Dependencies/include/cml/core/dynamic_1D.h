/* -*- C++ -*- ------------------------------------------------------------
 
Copyright (c) 2007 Jesse Anders and Demian Nave http://cmldev.net/

The Configurable Math Library (CML) is distributed under the terms of the
Boost Software License, v1.0 (see cml/LICENSE for details).

 *-----------------------------------------------------------------------*/
/** @file
 *  @brief
 */

#ifndef dynamic_1D_h
#define dynamic_1D_h

#include <vector>
#include <cml/core/common.h>
#include <cml/dynamic.h>

namespace cml {

/** Dynamically-sized and allocated 1D array.
 *
 * @note The allocator should be an STL-compatible allocator.
 *
 * @internal The internal array type <em>must</em> have the proper copy
 * semantics, otherwise copy construction will fail.
 */
template<typename Element, class Alloc>
class dynamic_1D
{
  public:

    /* Record the allocator type: */
    typedef typename Alloc::template rebind<Element>::other allocator_type;

    /* Record the generator: */
    typedef dynamic<Alloc> generator_type;

    /* Array implementation: */
    typedef std::vector<Element,allocator_type> array_impl;

    /* Standard: */
    typedef typename array_impl::value_type value_type;
    typedef typename array_impl::pointer pointer; 
    typedef typename array_impl::reference reference; 
    typedef typename array_impl::const_reference const_reference; 
    typedef typename array_impl::const_pointer const_pointer; 

    /* For matching by memory type: */
    typedef dynamic_memory_tag memory_tag;

    /* For matching by size type: */
    typedef dynamic_size_tag size_tag;

    /* For matching by resizability: */
    typedef resizable_tag resizing_tag;

    /* For matching by dimensions: */
    typedef oned_tag dimension_tag;


  public:

    /** Dynamic arrays have no fixed size. */
    enum { array_size = -1 };


  public:

    /** Construct a dynamic array with no size. */
    dynamic_1D() {}

    /** Construct a dynamic array given the size. */
    explicit dynamic_1D(size_t size) : m_data(size) {}


  public:

    /** Return the number of elements in the array. */
    size_t size() const { return this->m_data.size(); }

    /** Access to the data as a C array.
     *
     * @param i a size_t index into the array.
     * @return a mutable reference to the array value at i.
     *
     * @note This function does not range-check the argument.
     */
    reference operator[](size_t i) { return this->m_data[i]; }

    /** Const access to the data as a C array.
     *
     * @param i a size_t index into the array.
     * @return a const reference to the array value at i.
     *
     * @note This function does not range-check the argument.
     */
    const_reference operator[](size_t i) const { return this->m_data[i]; }

    /** Return access to the data as a raw pointer. */
    pointer data() { return &m_data[0]; }

    /** Return access to the data as a raw pointer. */
    const_pointer data() const { return &m_data[0]; }


  public:

    /** Set the array size to the given value.
     *
     * @warning This is not guaranteed to preserve the original data.
     */
    void resize(size_t s) { this->m_data.resize(s); }


  protected:

    array_impl                  m_data;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp
