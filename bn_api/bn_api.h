#include "intx/intx.hpp"
#include "src/interp/interp.h"

using namespace wabt;
using namespace wabt::interp;

class BNAPI {
private:
    wabt::interp::Memory *memory;
    interp::Result bnHostFunc(const HostFunc* func,
                                     const interp::FuncSignature* sig,
                                     const TypedValues& args,
                                     TypedValues& results);
public:
    BNAPI(wabt::interp::Memory *memory, interp::HostModule *host_module);

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
