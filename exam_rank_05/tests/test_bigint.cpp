#include "bigint.hpp"
#include <iostream>
#include <sstream>

static int pass = 0;
static int fail = 0;

#define CHECK(cond, name) do { \
    if (cond) { pass++; } \
    else { fail++; std::cout << "[FAIL] " << name << std::endl; } \
} while (0)

static std::string tostr(const bigint &b) {
    std::ostringstream os;
    os << b;
    return os.str();
}

int main() {
    // construction
    CHECK(tostr(bigint(42)) == "42", "construct from int");
    CHECK(tostr(bigint(0)) == "0", "construct zero");
    CHECK(tostr(bigint(1337)) == "1337", "construct 1337");
    bigint c;
    CHECK(tostr(c) == "0", "default construct zero");

    // addition
    CHECK(tostr(bigint(42) + bigint(21)) == "63", "addition");
    CHECK(tostr(bigint(999) + bigint(1)) == "1000", "addition carry");
    CHECK(tostr(bigint(0) + bigint(0)) == "0", "addition zero");
    bigint add(5);
    add += bigint(7);
    CHECK(tostr(add) == "12", "operator+=");

    // addition with int (subject main uses bigint + int)
    CHECK(tostr((bigint(23) << 9) + 42) == "23000000042", "bigint + int");

    // digitshift
    CHECK(tostr(bigint(42) << 3) == "42000", "left shift");
    CHECK(tostr(bigint(1337) >> 2) == "13", "right shift");
    CHECK(tostr(bigint(1337) >> 4) == "0", "right shift to zero");
    bigint sh(1337);
    sh <<= 4;
    CHECK(tostr(sh) == "13370000", "operator<<=");
    bigint sh2(13370000);
    sh2 >>= bigint(2);
    CHECK(tostr(sh2) == "133700", "operator>>= bigint");

    // increment / decrement
    bigint bi(21);
    bigint pre = ++bi;
    CHECK(tostr(bi) == "22" && tostr(pre) == "22", "operator++ pre");
    bigint bi2(22);
    bigint post = bi2++;
    CHECK(tostr(bi2) == "23" && tostr(post) == "22", "operator++ post");
    bigint bi3(5);
    bigint dpre = --bi3;
    CHECK(tostr(bi3) == "4" && tostr(dpre) == "4", "operator-- pre");
    bigint bi4(4);
    bigint dpost = bi4--;
    CHECK(tostr(bi4) == "3" && tostr(dpost) == "4", "operator-- post");
    bigint zero;
    --zero;
    CHECK(tostr(zero) == "0", "decrement zero stays zero");

    // comparison
    CHECK(bigint(42) < bigint(1337), "operator<");
    CHECK(bigint(42) <= bigint(42), "operator<=");
    CHECK(bigint(1337) > bigint(42), "operator>");
    CHECK(bigint(42) >= bigint(42), "operator>=");
    CHECK(bigint(42) == bigint(42), "operator==");
    CHECK(bigint(42) != bigint(1337), "operator!=");
    CHECK(bigint(1000) > bigint(999), "compare carry result");

    // bonus: subtraction (unsigned -> 0 if negative)
    CHECK(tostr(bigint(10) - bigint(3)) == "7", "subtraction bonus");
    CHECK(tostr(bigint(3) - bigint(10)) == "0", "subtraction negative -> 0");

    // bonus: multiplication
    CHECK(tostr(bigint(42) * bigint(21)) == "882", "multiplication bonus");
    CHECK(tostr(bigint(999) * bigint(999)) == "998001", "multiplication large");
    CHECK(tostr(bigint(123456789) * bigint(987654321)) ==
          "121932631112635269", "multiplication huge");

    // bonus: division / modulo by int
    CHECK(tostr(bigint(100) / 3) == "33", "division by int bonus");
    CHECK(tostr(bigint(100) % 3) == "1", "modulo by int bonus");

    // no leading zeros
    CHECK(tostr(bigint(0007)) == "7", "no leading zeros");

    // shift by zero
    CHECK(tostr(bigint(42) << 0) == "42", "left shift by zero");
    CHECK(tostr(bigint(42) >> 0) == "42", "right shift by zero");

    // shift by large amount
    CHECK(tostr((bigint(1) << 10)) == "10000000000", "left shift 1 by 10");
    CHECK(tostr((bigint(10) << 10 >> 10)) == "10", "right shift 10 by 10");

    // self-addition chain
    bigint sa(1);
    sa += sa;
    sa += sa;
    CHECK(tostr(sa) == "4", "double via +=");

    // multiplication by 1 and 0
    CHECK(tostr(bigint(12345) * bigint(1)) == "12345", "mul by 1");
    CHECK(tostr(bigint(12345) * bigint(0)) == "0", "mul by 0");

    // division edge cases
    CHECK(tostr(bigint(7) / 2) == "3", "small division");
    CHECK(tostr(bigint(1) / 3) == "0", "division rounds down");
    CHECK(tostr(bigint(0) / 5) == "0", "zero division");

    // modulo edge cases
    CHECK(tostr(bigint(10) % 5) == "0", "modulo exact");
    CHECK(tostr(bigint(1) % 100) == "1", "modulo smaller than divisor");

    std::cout << "\nbigint: " << pass << " passed, " << fail << " failed" << std::endl;
    return fail == 0 ? 0 : 1;
}
