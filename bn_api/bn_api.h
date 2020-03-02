#ifndef BNAPI_H
#define BNAPI_H
#include "intx/intx.hpp"
#include "src/interp/interp.h"

using namespace wabt;
using namespace wabt::interp;

extern intx::uint256 FqModulus;
extern intx::uint256 FqInv;
extern intx::uint256 FqRsquared;
extern intx::uint256 FrModulus;
extern intx::uint256 FrInv;
extern intx::uint256 FrRsquared;

void montgomery_multiplication_256(uint64_t* _a, uint64_t* _b, uint64_t* _mod, uint64_t* _inv, uint64_t* _out);
void montgomery_multiplication_256_interleaved(uint64_t* const x, uint64_t* const y, uint64_t* const m, uint64_t* const inv, uint64_t *out);

class BNAPI {
private:
    wabt::interp::Memory *memory;
    interp::Result bnHostFunc(const HostFunc* func,
                                     const interp::FuncSignature* sig,
                                     const TypedValues& args,
                                     TypedValues& results);
public:
    void SetMemory(wabt::interp::Memory *memory);
    void AddHostFunctions(interp::HostModule *host_module);

	void mul256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void div256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &c_offset, uint32_t &ret_offset);
    uint32_t add256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    uint32_t sub256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);

    void f1m_add(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void f1m_sub(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void f1m_mul(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void f1m_square(uint32_t &a_offset, uint32_t &ret_offset);
    void f1m_fromMont(uint32_t &a_offset, uint32_t &ret_offset);
    void f1m_toMont(uint32_t &a_offset, uint32_t &ret_offset);

    void fr_add(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void fr_sub(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void fr_mul(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset);
    void fr_square(uint32_t &a_offset, uint32_t &ret_offset);
    void fr_fromMont(uint32_t &a_offset, uint32_t &ret_offset);
    void fr_toMont(uint32_t &a_offset, uint32_t &ret_offset);
};
#endif
