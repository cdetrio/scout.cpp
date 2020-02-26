#include "bn_api.h"

#include <iostream>
#include <iomanip>

using namespace wabt;
using namespace wabt::interp;

typedef unsigned __int128 uint128_t;

const bool tracing = false;

intx::uint256 BignumOne = intx::from_string<intx::uint256>("1");

intx::uint256 FqModulus = intx::from_string<intx::uint256>("21888242871839275222246405745257275088696311157297823662689037894645226208583");
intx::uint256 FqInv = intx::from_string<intx::uint256>("211173256549385567650468519415768310665");
intx::uint256 FqRsquared = intx::from_string<intx::uint256>("3096616502983703923843567936837374451735540968419076528771170197431451843209");

intx::uint256 FrModulus = intx::from_string<intx::uint256>("21888242871839275222246405745257275088548364400416034343698204186575808495617");
intx::uint256 FrInv = intx::from_string<intx::uint256>("134950519161967129512891662437158223871");
intx::uint256 FrRsquared = intx::from_string<intx::uint256>("944936681149208446651664254269745548490766851729442924617792859073125903783");

intx::uint256 two_pow_256 = intx::from_string<intx::uint256>("115792089237316195423570985008687907853269984665640564039457584007913129639935");

void trace_words(uint8_t *mem, size_t len) {
    std::cout << std::hex << std::setw(2) << std::setfill('0');
    for (auto i = 0; i < len; i++) {
        for (auto j = 0; j < 32; j++) {
            std::cout << static_cast<int>(*(mem + j));
        }
        std::cout << std::dec << std::endl << std::hex;
    }

    std::cout << std::dec << std::endl;
}

static void trace_word(uint8_t *mem) {
    std::cout << std::hex << std::setw(2) << std::setfill('0');
    for (auto j = 0; j < 32; j++) {
        std::cout << static_cast<int>(*(mem + j));
    }

    std::cout << std::dec << std::endl;
}

//non-interleaved

void montgomery_multiplication_256(uint64_t* _a, uint64_t* _b, uint64_t* _mod, uint64_t* _inv, uint64_t* _out) {
  using intx::uint512;

  intx::uint256 *a = reinterpret_cast<intx::uint256 *>(_a);
  intx::uint256 *b = reinterpret_cast<intx::uint256 *>(_b);
  intx::uint256 *mod = reinterpret_cast<intx::uint256 *>(_mod);
  intx::uint256 *inv = reinterpret_cast<intx::uint256 *>(_inv);
  intx::uint256 *out = reinterpret_cast<intx::uint256 *>(_out);

  auto mask128 = intx::from_string<intx::uint128>("340282366920938463463374607431768211455");
  auto res1 = uint512{*a} * uint512{*b};
  //auto k0 = ((inv * res1).lo).lo;
  auto k0 = (uint512{*inv} * res1).lo & mask128;
  auto res2 = ((uint512{k0} * uint512{*mod}) + res1) >> 128;
  auto k1 = (res2 * uint512{*inv}).lo & mask128;
  auto result = ((uint512{k1} * uint512{*mod}) + res2) >> 128; // correct version
  //auto result = (((uint512{k1} * uint512{*mod}) + res2) >> 128).lo; // buggy version
  if (result >= *mod) {
    result = result - *mod;
  }
  *out = result.lo; // correct version
  //*out = result; // buggy version
}

/*

// interleaved (broken)

void montgomery_multiplication_256(uint64_t* const x, uint64_t* const y, uint64_t* const m, uint64_t* const inv, uint64_t *out){
  uint64_t A[256/64*2] = {0};
  for (int i=0; i<256/64; i++){
    uint64_t ui = (A[i]+x[i]*y[0])*inv[0];
    uint64_t carry = 0;
#pragma unroll
    for (int j=0; j<256/64; j++){
      __uint128_t xiyj = (__uint128_t)x[i]*y[j];
      __uint128_t uimj = (__uint128_t)ui*m[j];
      __uint128_t partial_sum = xiyj+carry+A[i+j];
      __uint128_t sum = uimj+partial_sum;
      A[i+j] = (uint64_t)sum;
      carry = sum>>64;

      if (sum<partial_sum){
        int k=2;
        while ( i+j+k<256/64*2 && A[i+j+k]==(uint64_t)0-1 ){
          A[i+j+k]=0;
          k++;
        }
        if (i+j+k<9)
          A[i+j+k]+=1;
      }

    }
    A[i+256/64]+=carry;
  }
  for (int i=0; i<=256/64;i++)
    out[i] = A[i+256/64];
  // check if m <= out
  int leq = 1;
  for (int i=256/64;i>=0;i--){
    if (m[i]>out[i]){
      leq = 0;
      break;
    }
    else if (m[i]<out[i]){
      break;
    }
  }
  // if leq, then perform final subtraction
  if (leq){
    uint64_t carry=0;
#pragma unroll
    for (int i=0; i<=256/64;i++){
      uint64_t temp = m[i]-carry;
      out[i] = temp-out[i];
      carry = (temp<out[i] || m[i]<carry) ? 1:0;
    }
  }

}
*/

void BNAPI::SetMemory(wabt::interp::Memory *memory) {
    this->memory = memory;
}

void BNAPI::AddHostFunctions(wabt::interp::HostModule *host_module) {
	host_module->AppendFuncExport("bignum_f1m_mul", {{Type::I32, Type::I32, Type::I32}, {}}, 
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->f1m_mul(arg1_offset, arg2_offset, ret_offset);
            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_f1m_square", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->f1m_square(arg_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_f1m_add", {{Type::I32, Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->f1m_add(arg1_offset, arg2_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_f1m_sub", {{Type::I32, Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->f1m_sub(arg1_offset, arg2_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_f1m_toMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->f1m_toMont(arg1_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_f1m_fromMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->f1m_fromMont(arg1_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_frm_mul", {{Type::I32, Type::I32, Type::I32}, {}}, 
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->fr_mul(arg1_offset, arg2_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_frm_square", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->fr_square(arg_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_frm_add", {{Type::I32, Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->fr_add(arg1_offset, arg2_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_frm_sub", {{Type::I32, Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->fr_sub(arg1_offset, arg2_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_frm_fromMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->fr_fromMont(arg1_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_frm_toMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->fr_toMont(arg1_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_int_mul", {{Type::I32, Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            this->mul256(arg1_offset, arg2_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_int_add", {{Type::I32, Type::I32, Type::I32}, {Type::I32}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);

            uint32_t result = this->add256(arg1_offset, arg2_offset, ret_offset);

            results[0].set_i32(result);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_int_sub", {{Type::I32, Type::I32, Type::I32}, {Type::I32}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[2].value.i32);
 
            uint32_t result = this->sub256(arg1_offset, arg2_offset, ret_offset);

            results[0].set_i32(result);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_int_div", {{Type::I32, Type::I32, Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t arg2_offset = static_cast<uint32_t>(args[1].value.i32);
            uint32_t carry_offset = static_cast<uint32_t>(args[2].value.i32);
            uint32_t result_offset = static_cast<uint32_t>(args[3].value.i32);

            this->div256(arg1_offset, arg2_offset, carry_offset, result_offset);

            return interp::ResultType::Ok;
        });
}

void BNAPI::mul256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    //std::cout << "int_mul " << intx::to_string(*a) << " * " << intx::to_string(*b);
	*ret_mem = (*a * *b ) % two_pow_256;
    //std::cout << " = " << intx::to_string(*ret_mem) << std::endl;
}

void BNAPI::div256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &c_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
	intx::uint256* ret_remainder_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));
	intx::uint256* ret_quotient_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[c_offset]));

	const auto div_res = udivrem(*a, *b);
	*ret_quotient_mem = div_res.quot;
	*ret_remainder_mem = div_res.rem;
}

uint32_t BNAPI::add256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
        intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
        intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
        intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

        const auto add_res = add_with_carry(*a, *b);

        *ret_mem = std::get<0>(add_res);

        if (tracing) {
            //std::cout << "add256 " << intx::to_string(*a) << " + " << intx::to_string(*b) << " = " << intx::to_string(*ret_mem) << "\n";
        }

        uint32_t carry = 0;
        if (std::get<1>(add_res) > 0) {
          carry = 1;
        }

        return carry;
}

uint32_t BNAPI::sub256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

	uint32_t carry = 0;
	if (*a < *b) {
	  carry = 1;
	}

	*ret_mem = *a - *b;

    return carry;
}

void add_mod(intx::uint256 *a, intx::uint256 *b, intx::uint256 *modulus, intx::uint256 *ret_mem) {
	intx::uint512 ret_full = intx::uint512{0,*a} + intx::uint512{0,*b};

	if (ret_full >= *modulus) {
	  ret_full -= intx::uint512{0, *modulus};
	}

	*ret_mem = ret_full.lo;
}

void sub_mod(intx::uint256 *a, intx::uint256 *b, intx::uint256 *modulus, intx::uint256 *ret_mem) {
    if (*a < *b) {
      *ret_mem = (*a + *modulus) - *b;
    } else {
      *ret_mem = *a - *b;
    }
}

void mul_mod(uint64_t *a, uint64_t *b, intx::uint256 *modulus, intx::uint256 *inverse, uint64_t *ret) {
    uint64_t *mod = reinterpret_cast<uint64_t *>(modulus);
    uint64_t *inv = reinterpret_cast<uint64_t *>(inverse);
    montgomery_multiplication_256(a, b, mod, inv, ret);
}

void square_mod(uint64_t *a, intx::uint256 *modulus, intx::uint256 *inverse, uint64_t *ret) {
    uint64_t *mod = reinterpret_cast<uint64_t *>(modulus);
    uint64_t *inv = reinterpret_cast<uint64_t *>(inverse);

    montgomery_multiplication_256(a, a, mod, inv, ret);
}

void fromMont(uint64_t *a, intx::uint256 *modulus, intx::uint256 *inverse, uint64_t *ret_offset) {
    uint64_t* b = reinterpret_cast<uint64_t*>(&BignumOne);
    uint64_t *mod = reinterpret_cast<uint64_t *>(modulus);
    uint64_t *inv = reinterpret_cast<uint64_t *>(inverse);

    montgomery_multiplication_256(a, b, mod, inv, ret_offset);
}

void toMont(uint64_t *a, intx::uint256 *r2, intx::uint256 *modulus, intx::uint256 *inverse, uint64_t *ret_offset) {
    uint64_t *mod = reinterpret_cast<uint64_t *>(modulus);
    uint64_t *inv = reinterpret_cast<uint64_t *>(inverse);
    uint64_t *r_squared = reinterpret_cast<uint64_t *>(r2);

    montgomery_multiplication_256(a, r_squared, mod, inv, ret_offset);
}

void BNAPI::f1m_add(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    if (tracing) {
        std::cout << "f1m_add " << intx::to_string(*a) << " + " << intx::to_string(*b);
    }

    add_mod(a, b, &FqModulus, ret_mem);

    if (tracing) {
        std::cout <<  " = " << intx::to_string(*ret_mem) << std::endl;
    }
}

void BNAPI::f1m_sub(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
    intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
    intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    if (tracing) {
        std::cout << "f1m_sub" << intx::to_string(*a) << " - " << intx::to_string(*b);
    }

    sub_mod(a, b, &FqModulus, ret_mem);

    if (tracing) {
        std::cout << " = " << intx::to_string(*ret_mem) << std::endl;
    }
}

void BNAPI::f1m_mul(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* b = reinterpret_cast<uint64_t*>(&(this->memory->data[b_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        intx::uint256 *b_num = reinterpret_cast<intx::uint256 *>(b);
        //std::cout << "f1m_mul " << intx::to_string(*a_num) << " * " << intx::to_string(*b_num);
    }

    mul_mod(a, b, &FqModulus, &FqInv, ret);

    if (tracing) {
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);
        //std::cout << " = " << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::f1m_square(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* mod = reinterpret_cast<uint64_t*>(&FqModulus);
    uint64_t* inv = reinterpret_cast<uint64_t*>(&FqInv);
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        std::cout << "f1m_square " << intx::to_string(*a_num) << " ** 2 = ";
    }

    square_mod(a, &FqModulus, &FqInv, ret);

    if (tracing) {
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);
        std::cout << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::f1m_fromMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        std::cout << "f1m_fromMont " << intx::to_string(*a_num);
    }

    fromMont(a, &FqModulus, &FqInv, ret);
    if (tracing) {
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);
        std::cout << " -> " << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::f1m_toMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);

        std::cout << "f1m_toMont " << intx::to_string(*a_num) << " -> ";
    }

    toMont(a, &FqRsquared, &FqModulus, &FqInv, ret);

    if (tracing) {
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);
        std::cout << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::fr_add(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    add_mod(a, b, &FrModulus, ret_mem);

    if (tracing) {
        std::cout << "fr_add " << intx::to_string(*a) << " - " << intx::to_string(*b) << " = " << intx::to_string(*ret_mem) << std::endl;
    }
}

void BNAPI::fr_sub(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
    intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

    intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));
    sub_mod(a, b, &FrModulus, ret_mem);

    if (tracing) {
        std::cout << "fr_sub " << intx::to_string(*a) << " - " << intx::to_string(*b) << " = " << intx::to_string(*ret_mem) << std::endl;
    }
}

void BNAPI::fr_mul(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* b = reinterpret_cast<uint64_t*>(&(this->memory->data[b_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    mul_mod(a, b, &FrModulus, &FrInv, ret);

    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        intx::uint256 *b_num = reinterpret_cast<intx::uint256 *>(b);
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);

        std::cout << "fr_mul " << intx::to_string(*a_num) << " * " << intx::to_string(*b_num) << " = " << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::fr_square(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    square_mod(a, &FrModulus, &FrInv, ret);
    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);

        std::cout << "fr_square " << intx::to_string(*a_num) << " ** 2 = " << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::fr_fromMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    fromMont(a, &FrModulus, &FrInv, ret);
    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);

        std::cout << "fr_fromMont " << intx::to_string(*a_num) << " -> " << intx::to_string(*ret_num) << std::endl;
    }
}

void BNAPI::fr_toMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data.data()[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));


    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        std::cout << "fr_toMont " << a_offset << " " << intx::to_string(*a_num);;
    }

    toMont(a, &FrRsquared, &FrModulus, &FrInv, ret);

    if (tracing) {
        intx::uint256 *a_num = reinterpret_cast<intx::uint256 *>(a);
        intx::uint256 *ret_num = reinterpret_cast<intx::uint256 *>(ret);

        std::cout << " -> " << intx::to_string(*ret_num) << std::endl;
        //std::cout << "modulus " << intx::to_string(FrModulus) << " inv " << intx::to_string(FrInv) << std::endl;
        //std::cout << a_offset << std::endl;
        //std::cout << static_cast<void*>((uint8_t *)(this->memory->data.data() + 512096)) << std::endl;
        //trace_word((uint8_t *)(this->memory->data.data() + 512096));
    }
}
