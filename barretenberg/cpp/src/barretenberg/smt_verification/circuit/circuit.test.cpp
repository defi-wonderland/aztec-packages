#include <fstream>
#include <iostream>
#include <string>

#include "barretenberg/proof_system/circuit_builder/standard_circuit_builder.hpp"
#include "barretenberg/stdlib/primitives/field/field.hpp"
#include "barretenberg/smt_verification/circuit/circuit.hpp"

#include <gtest/gtest.h>

using namespace bb;

namespace {
auto& engine = numeric::get_debug_randomness();
}

using field_t = stdlib::field_t<StandardCircuitBuilder>;
using witness_t = stdlib::witness_t<StandardCircuitBuilder>;
using pub_witness_t = stdlib::public_witness_t<StandardCircuitBuilder>;

// Verify the circuit, that evaluates (2a)/(3b)
TEST(circuit, expressionTrue)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(witness_t(&builder, fr::random_element()));
    field_t b(witness_t(&builder, fr::random_element()));
    field_t c = (a + a) / (b + b + b);

    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");
    builder.set_variable_name(c.witness_index, "c");
    ASSERT_TRUE(builder.check_circuit());

    auto buf = builder.export_circuit();

    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);
    smt_solver::Solver s(circuit_info.modulus, { true, 0 });
    smt_circuit::Circuit<smt_terms::FFTerm> circuit(circuit_info, &s);
    smt_terms::FFTerm a1 = circuit["a"];
    smt_terms::FFTerm b1 = circuit["b"];
    smt_terms::FFTerm c1 = circuit["c"];
    smt_terms::FFTerm cr = smt_terms::FFTerm::Var("cr", &s);
    cr = (2 * a1) / (2 * b1 + b1);
    c1 != cr;

    bool res = s.check();
    ASSERT_FALSE(res);
}

// Verify the circuit, that evaluates (2a)/(3b) using only multiplication.
TEST(circuit, multiplicationTrueKind)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(witness_t(&builder, fr::random_element()));
    field_t b(witness_t(&builder, fr::random_element()));
    field_t c = (a + a) / (b + b + b);

    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");
    builder.set_variable_name(c.witness_index, "c");
    ASSERT_TRUE(builder.check_circuit());

    auto buf = builder.export_circuit();

    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);
    smt_solver::Solver s(circuit_info.modulus, { true, 0 });
    smt_circuit::Circuit<smt_terms::FFTerm> circuit(circuit_info, &s);

    smt_terms::FFTerm a1 = circuit["a"];
    smt_terms::FFTerm b1 = circuit["b"];
    smt_terms::FFTerm c1 = circuit["c"];
    smt_terms::FFTerm cr = smt_terms::FFTerm::Var("cr", &s);
    cr * 3 * b1 == 2 * a1;
    c1 != cr;

    bool res = s.check();
    ASSERT_FALSE(res);
}

// Prove that the circuit, that should evaluate (2a) / (3b), is not correct.
TEST(circuit, multiplicationFalse)
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(witness_t(&builder, fr::random_element()));
    field_t b(witness_t(&builder, fr::random_element()));
    field_t c = (a) / (b + b + b); // mistake was here

    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");
    builder.set_variable_name(c.witness_index, "c");
    ASSERT_TRUE(builder.check_circuit());

    auto buf = builder.export_circuit();

    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);
    smt_solver::Solver s(circuit_info.modulus, { true, 0 });
    smt_circuit::Circuit<smt_terms::FFTerm> circuit(circuit_info, &s);

    smt_terms::FFTerm a1 = circuit["a"];
    smt_terms::FFTerm b1 = circuit["b"];
    smt_terms::FFTerm c1 = circuit["c"];

    smt_terms::FFTerm cr = smt_terms::FFTerm::Var("cr", &s);
    cr = (2 * a1) / (3 * b1);
    c1 != cr;

    bool res = s.check();
    ASSERT_TRUE(res);

    std::unordered_map<std::string, cvc5::Term> terms({ { "a", a1 }, 
                                                        { "b", b1 }, 
                                                        { "c", c1 }, 
                                                        { "cr", cr } });

    std::unordered_map<std::string, std::string> vals = s.model(terms);

    info("a = ", vals["a"]);
    info("b = ", vals["b"]);
    info("c = ", vals["c"]);
    info("c_res = ", vals["cr"]);
    info();
    info();

    // print the whole witness
    std::vector<cvc5::Term> vterms(circuit.symbolic_vars.cbegin(), circuit.symbolic_vars.cend());
    std::unordered_map<std::string, std::string> mmap = s.model(vterms);
    for(auto &x : circuit.symbolic_vars){
        std::string vname = x;
        info(vname, " = ", mmap[vname]);
    }
}

// Verify that the point is not unique during quadratic polynomial evaluation
// using unique_witness function
TEST(circuit, uniqueWitness)
// two roots of a quadratic eq x^2 + a * x + b = s
{
    StandardCircuitBuilder builder = StandardCircuitBuilder();

    field_t a(pub_witness_t(&builder, fr::random_element()));
    field_t b(pub_witness_t(&builder, fr::random_element()));
    builder.set_variable_name(a.witness_index, "a");
    builder.set_variable_name(b.witness_index, "b");
    field_t z(witness_t(&builder, fr::random_element()));
    field_t ev = z * z + a * z + b;
    builder.set_variable_name(z.witness_index, "z");
    builder.set_variable_name(ev.witness_index, "ev");

    auto buf = builder.export_circuit();

    smt_circuit::CircuitSchema circuit_info = smt_circuit::unpack_from_buffer(buf);
    smt_solver::Solver s(circuit_info.modulus, { true, 0 });

    std::pair<smt_circuit::Circuit<smt_terms::FFTerm>, smt_circuit::Circuit<smt_terms::FFTerm>> cirs =
        smt_circuit::unique_witness_ext<smt_terms::FFTerm>(circuit_info, &s, { "ev" }, { "z" }, {}, {}); // c1.ev == c2.ev, c1.z != c2.z

    bool res = s.check();
    ASSERT_TRUE(res);

    std::unordered_map<std::string, cvc5::Term> terms = { { "z_c1", cirs.first["z"] }, { "z_c2", cirs.second["z"] } };
    std::unordered_map<std::string, std::string> vals = s.model(terms);
    ASSERT_NE(vals["z_c1"], vals["z_c2"]);

    // print the whole witness
    info();
    info();
    std::vector<cvc5::Term> vterms1(cirs.first.symbolic_vars.cbegin(), cirs.first.symbolic_vars.cend());
    std::unordered_map<std::string, std::string> mmap = s.model(vterms1);
    for(auto &x : cirs.first.symbolic_vars){
        std::string vname = x;
        info(vname, " = ", mmap[vname]);
    }
    info();
    std::vector<cvc5::Term> vterms2(cirs.second.symbolic_vars.cbegin(), cirs.second.symbolic_vars.cend());
    mmap = s.model(vterms2);
    for(auto &x : cirs.second.symbolic_vars){
        std::string vname = x;
        info(vname, " = ", mmap[vname]);
    }
}
// TODO(alex): Try setting the whole witness to be not equal at the same time, while setting inputs and outputs to be 
// equal