/** @file
 *****************************************************************************

 Implementation of interfaces for wNAF ("weighted Non-Adjacent Form") exponentiation routines.

 See wnaf.hpp .

 *****************************************************************************
 * @author     This file is part of libff, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef WNAF_TCC_
#define WNAF_TCC_

#include <gmp.h>
#include <msvc_hack.h>

namespace libff {

template<mp_size_t n>
std::vector<ssize_t> find_wnaf(const size_t window_size, const bigint<n> &scalar)
{
    const size_t length = scalar.max_bits(); // upper bound
    std::vector<ssize_t> res(length+1);
    bigint<n> c = scalar;
    ssize_t j = 0;
    while (!c.is_zero())
    {
        ssize_t u;
        if ((c.data[0] & 1) == 1)
        {
            u = c.data[0] % (((size_t)1) << (window_size+1));
            if (u > ((ssize_t)1 << window_size))
            {
                u = u - ((ssize_t)1 << (window_size+1));
            }

            if (u > 0)
            {
                mpn_sub_1(c.data, c.data, n, u);
            }
            else
            {
                mpn_add_1(c.data, c.data, n, -u);
            }
        }
        else
        {
            u = 0;
        }
        res[j] = u;
        ++j;

        mpn_rshift(c.data, c.data, n, 1); // c = c/2
    }

    return res;
}

template<typename T, mp_size_t n>
T fixed_window_wnaf_exp(const size_t window_size, const T &base, const bigint<n> &scalar)
{
    std::vector<ssize_t> naf = find_wnaf(window_size, scalar);
    std::vector<T> table(((size_t)1) <<(window_size-1));
    T tmp = base;
    T dbl = base.dbl();
    for (size_t i = 0; i < ((size_t)1) <<(window_size-1); ++i)
    {
        table[i] = tmp;
        tmp = tmp + dbl;
    }

    T res = T::zero();
    bool found_nonzero = false;
    for (ssize_t i = naf.size()-1; i >= 0; --i)
    {
        if (found_nonzero)
        {
            res = res.dbl();
        }

        if (naf[i] != 0)
        {
            found_nonzero = true;
            if (naf[i] > 0)
            {
                res = res + table[naf[i]/2];
            }
            else
            {
                res = res - table[(-naf[i])/2];
            }
        }
    }

    return res;
}

template<typename T, mp_size_t n>
T opt_window_wnaf_exp(const T &base, const bigint<n> &scalar, const size_t scalar_bits)
{
    size_t best = 0;
    for (ssize_t i = T::wnaf_window_table.size() - 1; i >= 0; --i)
    {
        if (scalar_bits >= T::wnaf_window_table[i])
        {
            best = i+1;
            break;
        }
    }

    if (best > 0)
    {
        return fixed_window_wnaf_exp(best, base, scalar);
    }
    else
    {
        return scalar * base;
    }
}

} // libff

#endif // WNAF_TCC_
