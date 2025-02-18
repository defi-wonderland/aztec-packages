// AUTOGENERATED FILE
#pragma once

#include "../columns.hpp"
#include "barretenberg/relations/generic_lookup/generic_lookup_relation.hpp"

#include <cstddef>
#include <string_view>
#include <tuple>

namespace bb::avm2 {

/////////////////// lookup_bytecode_to_read_unary ///////////////////

class lookup_bytecode_to_read_unary_lookup_settings {
  public:
    static constexpr size_t READ_TERMS = 1;
    static constexpr size_t WRITE_TERMS = 1;
    static constexpr size_t READ_TERM_TYPES[READ_TERMS] = { 0 };
    static constexpr size_t WRITE_TERM_TYPES[WRITE_TERMS] = { 0 };
    static constexpr size_t LOOKUP_TUPLE_SIZE = 2;
    static constexpr size_t INVERSE_EXISTS_POLYNOMIAL_DEGREE = 4;
    static constexpr size_t READ_TERM_DEGREE = 0;
    static constexpr size_t WRITE_TERM_DEGREE = 0;

    // Columns using the Column enum.
    static constexpr Column SRC_SELECTOR = Column::bc_decomposition_sel;
    static constexpr Column DST_SELECTOR = Column::precomputed_sel_unary;
    static constexpr Column COUNTS = Column::lookup_bytecode_to_read_unary_counts;
    static constexpr Column INVERSES = Column::lookup_bytecode_to_read_unary_inv;
    static constexpr std::array<Column, LOOKUP_TUPLE_SIZE> SRC_COLUMNS = {
        Column::bc_decomposition_bytes_to_read, Column::bc_decomposition_bytes_to_read_unary
    };
    static constexpr std::array<Column, LOOKUP_TUPLE_SIZE> DST_COLUMNS = { Column::precomputed_clk,
                                                                           Column::precomputed_as_unary };

    template <typename AllEntities> static inline auto inverse_polynomial_is_computed_at_row(const AllEntities& in)
    {
        return (in.bc_decomposition_sel == 1 || in.precomputed_sel_unary == 1);
    }

    template <typename Accumulator, typename AllEntities>
    static inline auto compute_inverse_exists(const AllEntities& in)
    {
        using View = typename Accumulator::View;
        const auto is_operation = View(in.bc_decomposition_sel);
        const auto is_table_entry = View(in.precomputed_sel_unary);
        return (is_operation + is_table_entry - is_operation * is_table_entry);
    }

    template <typename AllEntities> static inline auto get_const_entities(const AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_nonconst_entities(AllEntities& in)
    {
        return get_entities(in);
    }

    template <typename AllEntities> static inline auto get_entities(AllEntities&& in)
    {
        return std::forward_as_tuple(in.lookup_bytecode_to_read_unary_inv,
                                     in.lookup_bytecode_to_read_unary_counts,
                                     in.bc_decomposition_sel,
                                     in.precomputed_sel_unary,
                                     in.bc_decomposition_bytes_to_read,
                                     in.bc_decomposition_bytes_to_read_unary,
                                     in.precomputed_clk,
                                     in.precomputed_as_unary);
    }
};

template <typename FF_>
class lookup_bytecode_to_read_unary_relation
    : public GenericLookupRelation<lookup_bytecode_to_read_unary_lookup_settings, FF_> {
  public:
    static constexpr std::string_view NAME = "LOOKUP_BYTECODE_TO_READ_UNARY";
};
template <typename FF_>
using lookup_bytecode_to_read_unary = GenericLookup<lookup_bytecode_to_read_unary_lookup_settings, FF_>;

} // namespace bb::avm2