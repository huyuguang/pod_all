#pragma once

#include <stdlib.h>
#include <iostream>

#include <libsnark/common/default_types/r1cs_ppzksnark_pp.hpp>
#include <libsnark/gadgetlib1/gadget.hpp>
#include <libsnark/gadgetlib1/gadgets/basic_gadgets.hpp>
#include <libsnark/gadgetlib1/pb_variable.hpp>
#include <libsnark/zk_proof_systems/ppzksnark/r1cs_ppzksnark/r1cs_ppzksnark.hpp>

template <typename FieldT>
class Mimc5Gadget : public libsnark::gadget<FieldT> {
 public:
  const std::vector<FieldT>& constants_;
  const libsnark::pb_linear_combination<FieldT> seed_;
  const libsnark::pb_variable<FieldT> digest_;
  libsnark::pb_variable_array<FieldT> rounds_x2_;
  libsnark::pb_variable_array<FieldT> rounds_x4_;
  libsnark::pb_variable_array<FieldT> rounds_x5_;

  Mimc5Gadget(libsnark::protoboard<FieldT>& pb,
                const std::vector<FieldT>& constants,
                const libsnark::pb_linear_combination<FieldT> seed,
                const libsnark::pb_variable<FieldT> digest,
                const std::string& annotation_prefix = "")
      : libsnark::gadget<FieldT>(pb, annotation_prefix),
        constants_(constants),
        seed_(seed),
        digest_(digest) {
    rounds_x2_.allocate(pb, constants_.size(),
                        FMT(annotation_prefix, " rounds_x2"));
    rounds_x4_.allocate(pb, constants_.size(),
                        FMT(annotation_prefix, " rounds_x4"));
    rounds_x5_.allocate(pb, constants_.size(),
                        FMT(annotation_prefix, " rounds_x5"));
  }

  libsnark::pb_variable<FieldT> result() {
    return rounds_x5_[constants_.size() - 1];
  }

  void generate_r1cs_constraints() {
    libsnark::pb_variable<FieldT> data;
    for (size_t i = 0; i < constants_.size(); ++i) {
      auto x1 = i == 0 ? (seed_ + constants_[i]) : (data + seed_ + constants_[i]);
      this->pb.add_r1cs_constraint(
          libsnark::r1cs_constraint<FieldT>(x1, x1, rounds_x2_[i]), "x1*x1");
      this->pb.add_r1cs_constraint(
          libsnark::r1cs_constraint<FieldT>(rounds_x2_[i], rounds_x2_[i],
                                        rounds_x4_[i]),
          "x2*x2");

      if (i < constants_.size() - 1) {
        this->pb.add_r1cs_constraint(
            libsnark::r1cs_constraint<FieldT>(rounds_x4_[i], x1, rounds_x5_[i]),
            "x4*x1");
        data = rounds_x5_[i];
      } else {
        this->pb.add_r1cs_constraint(
            libsnark::r1cs_constraint<FieldT>(rounds_x4_[i], x1,
                                          rounds_x5_[i] - seed_),
            "x5");
      }
    }

    // digest_ == x.back()
    this->pb.add_r1cs_constraint(libsnark::r1cs_constraint<FieldT>(
        1, rounds_x5_[constants_.size() - 1] - digest_, 0));
  }

  void generate_r1cs_witness() {
    seed_.evaluate(this->pb);
    FieldT seed = this->pb.lc_val(seed_);
    FieldT data = 0;
    for (size_t i = 0; i < constants_.size(); ++i) {
      auto x1 = data + seed + constants_[i];
      auto x2 = x1 * x1;
      auto x4 = x2 * x2;
      auto x5 = x4 * x1;
      this->pb.val(rounds_x2_[i]) = x2;
      this->pb.val(rounds_x4_[i]) = x4;
      if (i < constants_.size() - 1) {
        this->pb.val(rounds_x5_[i]) = x5;
        // std::cout << x5 << "\n";
        data = x5;
      } else {
        this->pb.val(rounds_x5_[i]) = x5 + seed;
      }
    }
  }
};