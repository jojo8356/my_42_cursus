#include "vect2.hpp"
#include <iostream>
#include <sstream>

static int pass = 0;
static int fail = 0;

#define CHECK(cond, name) do { \
    if (cond) { pass++; } \
    else { fail++; std::cout << "[FAIL] " << name << std::endl; } \
} while (0)

static std::string tostr(const vect2 &v) {
    std::ostringstream os;
    os << v;
    return os.str();
}

int main() {
    // construction
    CHECK(vect2(2, 2) * 2 == vect2(4, 4), "scalar multiplication");
    CHECK(vect2(1, 2) + vect2(3, 4) == vect2(4, 6), "addition");
    CHECK(vect2(5, 5) - vect2(2, 1) == vect2(3, 4), "subtraction");
    CHECK(vect2(4, 6) / 2 == vect2(2, 3), "scalar division (bonus)");

    // operator[]
    vect2 v(10, 20);
    CHECK(v[0] == 10 && v[1] == 20, "operator[] read");
    v[1] = 99;
    CHECK(v[1] == 99, "operator[] write");
    const vect2 cv(1, 2);
    CHECK(cv[0] == 1 && cv[1] == 2, "operator[] const read");

    // compound assign
    vect2 a(1, 2);
    a += vect2(3, 4);
    CHECK(a == vect2(4, 6), "operator+=");
    vect2 b(5, 5);
    b -= vect2(2, 1);
    CHECK(b == vect2(3, 4), "operator-=");
    vect2 c(2, 3);
    c *= 4;
    CHECK(c == vect2(8, 12), "operator*=");
    vect2 d(8, 12);
    d /= 2;
    CHECK(d == vect2(4, 6), "operator/=");

    // increment / decrement
    vect2 e(1, 2);
    vect2 pre = ++e;
    CHECK(e == vect2(2, 3) && pre == vect2(2, 3), "operator++ pre");
    vect2 f(2, 3);
    vect2 post = f++;
    CHECK(f == vect2(3, 4) && post == vect2(2, 3), "operator++ post");
    vect2 g(5, 5);
    vect2 gpre = --g;
    CHECK(g == vect2(4, 4) && gpre == vect2(4, 4), "operator-- pre");
    vect2 h(4, 4);
    vect2 hpost = h--;
    CHECK(h == vect2(3, 3) && hpost == vect2(4, 4), "operator-- post");

    // unary minus
    CHECK(-vect2(3, 4) == vect2(-3, -4), "unary minus");

    // comparison
    CHECK(vect2(1, 2) == vect2(1, 2), "operator==");
    CHECK(vect2(1, 2) != vect2(3, 4), "operator!=");

    // stream output
    CHECK(tostr(vect2(7, 8)) == "{7, 8}", "operator<< format");

    // copy / assignment
    vect2 src(3, 4);
    vect2 dst = src;
    CHECK(dst == src, "copy constructor");
    vect2 dst2;
    dst2 = src;
    CHECK(dst2 == src, "assignment operator");

    // negative / zero values
    CHECK(vect2(-1, -2) + vect2(3, 4) == vect2(2, 2), "addition with negatives");
    CHECK(vect2(0, 0) == vect2(0, 0), "zero vector equality");
    vect2 neg(-5, 10);
    neg[0] = -100;
    CHECK(neg[0] == -100, "negative index write");
    CHECK(-vect2(0, 5) == vect2(0, -5), "unary minus zero coord");

    // large values
    CHECK(vect2(1000000, 2000000) + vect2(1, 1) == vect2(1000001, 2000001),
          "large values addition");

    // division by 1 leaves unchanged
    CHECK(vect2(7, 9) / 1 == vect2(7, 9), "division by 1");

    // subject main behavior: v2 += v2 += v3
    // inner v2 += v3 runs first (v2=(2,4)), then v2 += v2 -> (4,8)
    vect2 v2(1, 2), v3(1, 2);
    v2 += v2 += v3;
    CHECK(v2 == vect2(4, 8), "chained +=");

    std::cout << "\nvect2: " << pass << " passed, " << fail << " failed" << std::endl;
    return fail == 0 ? 0 : 1;
}
