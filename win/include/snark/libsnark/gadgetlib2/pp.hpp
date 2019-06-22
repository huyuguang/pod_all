/** @file
 *****************************************************************************
 Declaration of PublicParams for Fp field arithmetic
 *****************************************************************************
 * @author     This file is part of libsnark, developed by SCIPR Lab
 *             and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef LIBSNARK_GADGETLIB2_INCLUDE_GADGETLIB2_PP_HPP_
#define LIBSNARK_GADGETLIB2_INCLUDE_GADGETLIB2_PP_HPP_

#include <memory>
#include <vector>
#include <msvc_hack.h>
#include <libff/common/default_types/ec_pp.hpp>

namespace gadgetlib2 {

/*************************************************************************************************/
/*************************************************************************************************/
/*******************                                                            ******************/
/*******************                        R1P World                           ******************/
/*******************                                                            ******************/
/*************************************************************************************************/
/*************************************************************************************************/

/* curve-specific public parameters */
typedef libff::Fr<libff::default_ec_pp> Fp;

typedef std::vector<Fp> FpVector;

class PublicParams {
public:
    size_t log_p;
    PublicParams(const std::size_t log_p);
    Fp getFp(ssize_t x) const; // to_support changes later
    ~PublicParams();
};

PublicParams initPublicParamsFromDefaultPp();

} // namespace gadgetlib2
#endif // LIBSNARK_GADGETLIB2_INCLUDE_GADGETLIB2_PP_HPP_
