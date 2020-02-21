#include "bn_api.h"

intx::uint256 FqModulus = intx::from_string<intx::uint256>("21888242871839275222246405745257275088696311157297823662689037894645226208583");
intx::uint256 FqInv = intx::from_string<intx::uint256>("211173256549385567650468519415768310665");
intx::uint256 FqRsquared = intx::from_string<intx::uint256>("3096616502983703923843567936837374451735540968419076528771170197431451843209");

intx::uint256 FrModulus = intx::from_string<intx::uint256>("21888242871839275222246405745257275088548364400416034343698204186575808495617");
intx::uint256 FrInv = intx::from_string<intx::uint256>("134950519161967129512891662437158223871");
intx::uint256 FrRsquared = intx::from_string<intx::uint256>("944936681149208446651664254269745548490766851729442924617792859073125903783");

BNAPI::BNAPI(Memory *memory) {
    this->memory = memory;
}

void BNAPI::mul256(uint64_t *a_offset, uint64_t* b_offset, uint64_t *ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

	*ret_mem = *a * *b;
}

uint32_t BNAPI::div256(uint64_t *a_offset, uint64_t *b_offset, uint64_t *c_offset, uint64_t *ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));
	intx::uint256* ret_remainder_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));
	intx::uint256* ret_quotient_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[c_offset]));

	const auto div_res = udivrem(*a, *b);
	*ret_quotient_mem = div_res.quot;
	*ret_remainder_mem = div_res.rem;
}

uint32_t BNAPI::add256(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset) {
        Memory* mem = &env_->memories_[0];
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

uint32_t BNAPI::sub256(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
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

void BNAPI::f1m_add(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset) {
	intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
	intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

	intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

	intx::uint512 ret_full = intx::uint512{0,*a} + intx::uint512{0,*b};

	if (ret_full >= FqModulus) {
	  ret_full -= intx::uint512{0, FqModulus};
	}

	*ret_mem = ret_full.lo;
}

void BNAPI::f1m_sub(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset) {
    intx::uint256* a = reinterpret_cast<intx::uint256*>(&(this->memory->data[a_offset]));
    intx::uint256* b = reinterpret_cast<intx::uint256*>(&(this->memory->data[b_offset]));

    intx::uint256* ret_mem = reinterpret_cast<intx::uint256*>(&(this->memory->data[ret_offset]));

    if (*a < *b) {
      *ret_mem = (*a + FqModulus) - *b;
    } else {
      *ret_mem = *a - *b;
    }
}

void BNAPI::f1m_mul(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
        uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
        uint64_t* b = reinterpret_cast<uint64_t*>(&(this->memory->data[b_offset]));

        uint64_t* mod = reinterpret_cast<uint64_t*>(&FqModulus);
        uint64_t* inv = reinterpret_cast<uint64_t*>(&FqInv);

        uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

        montgomery_multiplication_256(a, b, &FqModulus, inv, ret);
}

void BNAPI::f1m_square(uint64_t *a_offset, uint64_t *ret_offset);
        uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));

        uint64_t* mod = reinterpret_cast<uint64_t*>(&FqModulus);
        uint64_t* inv = reinterpret_cast<uint64_t*>(&FqInv);

        uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

        //std::cout << "square\n";
        montgomery_multiplication_256(a, a, &FqModulus, inv, ret);
}

void BNAPI::f1m_fromMont(uint64_t *a_offset, uint64_t *ret_offset) {
        uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
        uint64_t* b = reinterpret_cast<uint64_t*>(&wabt::interp::BignumOne);

        uint64_t* mod = reinterpret_cast<uint64_t*>(&FqModulus);
        uint64_t* inv = reinterpret_cast<uint64_t*>(&FqInv);

        uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

        montgomery_multiplication_256(a, b, &FqModulus, inv, ret);
}

void BNAPI::f1m_toMont(uint64_t *a_offset, uint64_t *ret_offset) {
        uint64_t* a = reinterpret_cast<uint64_t*>(&(this->memory->data[a_offset]));
        uint64_t* b = reinterpret_cast<uint64_t*>(&FqModulus);

        uint64_t* mod = reinterpret_cast<uint64_t*>(&FqModulus);
        uint64_t* inv = reinterpret_cast<uint64_t*>(&FqInv);

        uint64_t* ret = reinterpret_cast<uint64_t*>(&(this->memory->data[ret_offset]));

		// std::cout << "f1m_toMontgomery\na :" << format_u256_hex((uint8_t *)a) << "\nr squared: " << format_u256_hex((uint8_t *)b) << "\n modulus: " << format_u256_hex((uint8_t *)mod) << "\ninverse: " << format_u256_hex((uint8_t *)inv) << "\n";
        montgomery_multiplication_256(a, b, &FqModulus, inv, ret);
}
void BNAPI::fr_add(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset) {

}

void BNAPI::fr_sub(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset) {

}

void BNAPI::fr_mul(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset) {

}

void BNAPI::fr_square(uint64_t *a_offset, uint64_t *ret_offset) {

}

void BNAPI::fr_fromMont(uint64_t *a_offset, uint64_t *ret_offset) {

}

void BNAPI::fr_toMont(uint64_t *a_offset, uint64_t *ret_offset) {

}
