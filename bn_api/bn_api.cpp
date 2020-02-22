#include "bn_api.h"

#include <iostream>

using namespace wabt;
using namespace wabt::interp;

typedef unsigned __int128 uint128_t;

intx::uint256 BignumOne = intx::from_string<intx::uint256>("1");

intx::uint256 FqModulus = intx::from_string<intx::uint256>("21888242871839275222246405745257275088696311157297823662689037894645226208583");
intx::uint256 FqInv = intx::from_string<intx::uint256>("211173256549385567650468519415768310665");
intx::uint256 FqRsquared = intx::from_string<intx::uint256>("3096616502983703923843567936837374451735540968419076528771170197431451843209");

intx::uint256 FrModulus = intx::from_string<intx::uint256>("21888242871839275222246405745257275088548364400416034343698204186575808495617");
intx::uint256 FrInv = intx::from_string<intx::uint256>("134950519161967129512891662437158223871");
intx::uint256 FrRsquared = intx::from_string<intx::uint256>("944936681149208446651664254269745548490766851729442924617792859073125903783");

void montgomery_multiplication_256(uint64_t* x, uint64_t* y, uint64_t* modulus, uint64_t* inv, uint64_t* outOffset){

   //uint64_t A[] = {0,0,0,0,0,0,0,0};
   uint64_t A[] = {0,0,0,0,0,0,0,0,0};
   for (int i=0; i<4; i++){
     uint64_t ui = (A[i]+x[i]*y[0])*inv[0];
     uint64_t carry = 0;
     //uint64_t overcarry = 0;
     for (int j=0; j<4; j++){
       uint128_t xiyj = (uint128_t)x[i]*y[j];
       uint128_t uimj = (uint128_t)ui*modulus[j];
       uint128_t partial_sum = xiyj+carry;
       uint128_t sum = uimj+A[i+j]+partial_sum;
       A[i+j] = (uint64_t)sum;
       carry = sum>>64;
       // if there was overflow in the sum beyond the carry bits
       if (sum<partial_sum){
         int k=2;
         while ( i+j+k<8 && A[i+j+k]==0xffffffffffffffff ){
           A[i+j+k]=0;
           k++;
         }
         if (i+j+k<9)
           A[i+j+k]+=1;
       }
     }
     A[i+4]+=carry;
   }

   uint64_t out[] = {0,0,0,0,0};

   // instead of right shift, we just get the correct values
   out[0] = A[4];
   out[1] = A[5];
   out[2] = A[6];
   out[3] = A[7];
   out[4] = A[8];

    outOffset[0] = out[0];
    outOffset[1] = out[1];
    outOffset[2] = out[2];
    outOffset[3] = out[3];

    intx::uint256 *output = reinterpret_cast<intx::uint256*>(outOffset);
    if (*output > *modulus) {
        *output -= *modulus;
    }
}

interp::Result BNAPI::bnHostFunc(const HostFunc* func,
                                    const interp::FuncSignature* sig,
                                    const TypedValues& args,
                                    TypedValues& results) {
  std::cout << "called " << func->field_name << std::endl;
  std::string host_func = func->field_name;

  if (host_func == "bignum_f1m_mul") { 

  } else if (host_func == "bignum_f1m_square") {

  } else if (host_func == "bignum_f1m_add") {

  } else if (host_func == "bignum_f1m_sub") {

  } else if (host_func == "bignum_f1m_toMontgomery") {

  } else if (host_func == "bignum_f1m_fromMontgomery") {

  } else if (host_func == "bignum_int_mul") {

  } else if (host_func == "bignum_int_add") {

  } else if (host_func == "bignum_int_sub") {

  } else if (host_func == "bignum_int_div") {

  } else if (host_func == "bignum_fr_mul") { 
      std::cout << "fr_mul\n";
  } else if (host_func == "bignum_fr_square") {

  } else if (host_func == "bignum_fr_add") {

  } else if (host_func == "bignum_fr_sub") {

  } else if (host_func == "bignum_fr_toMontgomery") {

  } else if (host_func == "bignum_fr_fromMontgomery") {

  }

  return interp::ResultType::Ok;
}

BNAPI::BNAPI(wabt::interp::Memory *memory, wabt::interp::HostModule *host_module) {
    this->memory = memory;

	host_module->AppendFuncExport("bignum_f1m_mul", {{Type::I32, Type::I32, Type::I32}, {}}, 
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

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

	host_module->AppendFuncExport("bignum_fr_mul", {{Type::I32, Type::I32, Type::I32}, {}}, 
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_fr_square", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);
            this->fr_square(arg_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_fr_add", {{Type::I32, Type::I32, Type::I32}, {}},
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

	host_module->AppendFuncExport("bignum_fr_sub", {{Type::I32, Type::I32, Type::I32}, {}},
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

	host_module->AppendFuncExport("bignum_fr_toMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->fr_toMont(arg1_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_fr_fromMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->fr_fromMont(arg1_offset, ret_offset);

            return interp::ResultType::Ok;
        });

	host_module->AppendFuncExport("bignum_fr_fromMontgomery", {{Type::I32, Type::I32}, {}},
        [&]( const interp::HostFunc* host_func, 
             const interp::FuncSignature *signature,
             const interp::TypedValues &args,
             interp::TypedValues &results) {

            uint32_t arg1_offset = static_cast<uint32_t>(args[0].value.i32);
            uint32_t ret_offset = static_cast<uint32_t>(args[1].value.i32);

            this->fr_fromMont(arg1_offset, ret_offset);

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

    /*
	host_module->AppendFuncExport("bignum_int_sub", {{Type::I32, Type::I32, Type::I32}, {Type::I32}}, this->bnHostFunc);
	host_module->AppendFuncExport("bignum_int_div", {{Type::I32, Type::I32, Type::I32, Type::I32}, {}}, this->bnHostFunc);
	host_module->AppendFuncExport("bignum_frm_mul", {{Type::I32, Type::I32, Type::I32}, {}}, this->bnHostFunc);
    */
}

void BNAPI::mul256(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

	*ret_mem = *a * *b;
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

void toMont(uint64_t *a, intx::uint256 *modulus, intx::uint256 *inverse, uint64_t *ret_offset) {
    uint64_t *mod = reinterpret_cast<uint64_t *>(modulus);
    uint64_t *inv = reinterpret_cast<uint64_t *>(inverse);

    montgomery_multiplication_256(a, mod, mod, inv, ret_offset);
}

void BNAPI::f1m_add(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    add_mod(a, b, &FqModulus, ret_mem);
}

void BNAPI::f1m_sub(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
    intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
    intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    sub_mod(a, b, &FqModulus, ret_mem);
}

void BNAPI::f1m_mul(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* b = reinterpret_cast<uint64_t*>(&(this->memory->data[b_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    mul_mod(a, b, &FqModulus, &FqInv, ret);
}

void BNAPI::f1m_square(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* mod = reinterpret_cast<uint64_t*>(&FqModulus);
    uint64_t* inv = reinterpret_cast<uint64_t*>(&FqInv);
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    square_mod(a, &FqModulus, &FqInv, ret);
}

void BNAPI::f1m_fromMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    fromMont(a, &FqModulus, &FqInv, ret);
}

void BNAPI::f1m_toMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    toMont(a, &FqModulus, &FqInv, ret);
}

void BNAPI::fr_add(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    add_mod(a, b, &FrModulus, ret_mem);
}

void BNAPI::fr_sub(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
    intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

    intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));
    sub_mod(a, b, &FrModulus, ret_mem);

}

void BNAPI::fr_mul(uint32_t &a_offset, uint32_t &b_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* b = reinterpret_cast<uint64_t*>(&(this->memory->data[b_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    mul_mod(a, b, &FrModulus, &FrInv, ret);
}

void BNAPI::fr_square(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    square_mod(a, &FrModulus, &FrInv, ret);
}

void BNAPI::fr_fromMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    fromMont(a, &FrModulus, &FrInv, ret);
}

void BNAPI::fr_toMont(uint32_t &a_offset, uint32_t &ret_offset) {
    uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
    uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

    toMont(a, &FrModulus, &FrInv, ret);
}
