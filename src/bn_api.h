class BNAPI {
private:
    intx::uint256 fq_modulus;
    intx::uint256 fq_inv;
    intx::uint256 fq_rsquared;

    intx::uint256 fr_modulus;
    intx::uint256 fr_inv;
    intx::uint256 fr_rsquared;

    Memory *memory;
public:
	//BNAPI(memory, field_modulus, field_inv, field_rsquared);
	void mul256(uint64_t *a_offset, uint64_t* b_offset, uint64_t *ret_offset);
    uint32_t div256(uint64_t *a_offset, uint64_t *b_offset, uint64_t *c_offset, uint64_t *ret_offset);
    void add256(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void sub256(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);

    void f1m_add(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void f1m_sub(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void f1m_mul(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void f1m_square(uint64_t *a_offset, uint64_t *ret_offset);
    void f1m_fromMont(uint64_t *a_offset, uint64_t *ret_offset);
    void f1m_toMont(uint64_t *a_offset, uint64_t *ret_offset);

    void fr_add(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void fr_sub(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void fr_mul(uint64_t *a_offset, uint64_t *b_offset, uint64_t *ret_offset);
    void fr_square(uint64_t *a_offset, uint64_t *ret_offset);
    void fr_fromMont(uint64_t *a_offset, uint64_t *ret_offset);
    void fr_toMont(uint64_t *a_offset, uint64_t *ret_offset);
};
