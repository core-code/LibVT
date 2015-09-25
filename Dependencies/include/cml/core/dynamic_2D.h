/* -*- C++ -*- ------------------------------------------------------------
 
Copyright (c) 2007 Jesse Anders and Demian Nave http://cmldev.net/

The Configurable Math Library (CML) is distributed under the terms of the
Boost Software License, v1.0 (see cml/LICENSE for details).

 *-----------------------------------------------------------------------*/
/** @file
 *  @brief
 */

#ifndef dynamic_2D_h
#define dynamic_2D_h

#include <vector>
#include <cml/core/common.h>
#include <cml/core/dynamic_1D.h>
#include <cml/dynamic.h>

namespace cml {

/** Dynamically-sized and allocated 2D array.
 *
 * @note The allocator should be an STL-compatible allocator.
 *
 * @internal The internal array type <em>must</em> have the proper copy
 * semantics, otherwise copy construction will fail.
 *
 * @internal This class does not need a destructor.
 */
template<typename Element, typename Layout, class Alloc>
class dynamic_2D
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

    /* For matching by memory layout: */
    typedef Layout layout;

    /* For matching by memory type: */
    typedef dynamic_memory_tag memory_tag;

    /* For matching by size type: */
    typedef dynamic_size_tag size_tag;

    /* For matching by resizability: */
    typedef resizable_tag resizing_tag;

    /* For matching by dimensions: */
    typedef twod_tag dimension_tag;

    /* To simplify the matrix transpose operator: */
    typedef dynamic_2D<Element,Layout,Alloc> transposed_type;

    /* To simplify the matrix row and column operators: */
    typedef dynamic_1D<Element,Alloc> row_array_type;
    typedef dynamic_1D<Element,Alloc> col_array_type;


  protected:

    /** Construct a dynamic array with no size. */
    dynamic_2D() {}

    /** Construct a dynamic matrix given the dimensions.
     *
     * This constructor is guaranteed to throw only if the allocator throws.
     * If the array implementation guarantees that the array data structure is
     * not modified after an exception, then this constructor is
     * exception-safe.
     *
     * @throws only if the allocator throws during an allocation.
     */
    explicit dynamic_2D(size_t rows, size_t cols) 
        : m_rows(rows), m_cols(cols), m_data(rows*cols) {}


  public:

    enum { array_rows = -1, array_cols = -1 };


  public:

    /** Return the number of rows in the array. */
    size_t rows() const { return m_rows; }

    /** Return the number of cols in the array. */
    size_t cols() const { return m_cols; }


  public:

    /** Access the given element of the matrix.
     *
     * @param row row of element.
     * @param col column of element.
     * @returns mutable reference.
     */
    reference operator()(size_t row, size_t col) {
        return get_element(row, col, layout());
    }

    /** Access the given element of the matrix.
     *
     * @param row row of element.
     * @param col column of element.
     * @returns const reference.
     */
    const_reference operator()(size_t row, size_t col) const {
        return get_element(row, col, layout());
    }

    /** Return access to the data as a raw pointer. */
    pointer data() { return &m_data[0]; }

    /** Return access to the data as a raw pointer. */
    const_pointer data() const { return &m_data[0]; }


  public:

    /** Resize the array.
     *
     * @warning This is not guaranteed to preserve the original data.
     */
    void resize(size_t rows, size_t cols) {
        m_data.resize(rows*cols); m_rows = rows; m_cols = cols;
    }


  protected:

    reference get_element(size_t row, size_t col, row_major) {
        return m_data[row*m_cols + col];
    }

    const_reference get_element(size_t row, size_t col, row_major) const {
        return m_data[row*m_cols + col];
    }

    reference get_element(size_t row, size_t col, col_major) {
        return m_data[col*m_rows + row];
    }

    const_reference get_element(size_t row, size_t col, col_major) const {
        return m_data[col*m_rows + row];
    }


  protected:

    size_t                      m_rows, m_cols;
    array_impl                  m_data;
};

} // namespace cml

#endif

// -------------------------------------------------------------------------
// vim:ft=cpp
