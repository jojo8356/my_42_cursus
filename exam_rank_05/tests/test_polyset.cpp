#include "searchable_array_bag.hpp"
#include "searchable_tree_bag.hpp"
#include "set.hpp"
#include <iostream>

static int pass = 0;
static int fail = 0;

#define CHECK(cond, name) do { \
    if (cond) { pass++; } \
    else { fail++; std::cout << "[FAIL] " << name << std::endl; } \
} while (0)

int main() {
    // searchable_array_bag: insert + has
    searchable_array_bag abag;
    abag.insert(5);
    abag.insert(3);
    abag.insert(7);
    CHECK(abag.has(5) == true, "array_bag has existing");
    CHECK(abag.has(3) == true, "array_bag has existing 3");
    CHECK(abag.has(7) == true, "array_bag has existing 7");
    CHECK(abag.has(9) == false, "array_bag has missing");
    CHECK(abag.count() == 3, "array_bag count bonus");
    CHECK(abag.empty() == false, "array_bag not empty");

    // searchable_tree_bag: insert + has
    searchable_tree_bag tbag;
    tbag.insert(5);
    tbag.insert(3);
    tbag.insert(7);
    CHECK(tbag.has(5) == true, "tree_bag has existing");
    CHECK(tbag.has(3) == true, "tree_bag has existing 3");
    CHECK(tbag.has(7) == true, "tree_bag has existing 7");
    CHECK(tbag.has(9) == false, "tree_bag has missing");
    CHECK(tbag.empty() == false, "tree_bag not empty");

    // insert array
    int arr[] = {1, 2, 3, 4};
    searchable_array_bag abag2;
    abag2.insert(arr, 4);
    CHECK(abag2.has(1) && abag2.has(2) && abag2.has(3) && abag2.has(4),
          "array_bag insert array");
    CHECK(abag2.has(5) == false, "array_bag insert array no extra");

    // tree_bag insert array
    searchable_tree_bag tbag2;
    tbag2.insert(arr, 4);
    CHECK(tbag2.has(1) && tbag2.has(2) && tbag2.has(3) && tbag2.has(4),
          "tree_bag insert array");

    // set: anti-duplicate
    searchable_array_bag base;
    base.insert(1);
    base.insert(2);
    set s(base);
    s.insert(1);  // duplicate -> not inserted
    s.insert(3);
    CHECK(s.has(1) == true, "set has 1");
    CHECK(s.has(2) == true, "set has 2");
    CHECK(s.has(3) == true, "set has 3");

    // set with tree bag
    searchable_tree_bag base2;
    base2.insert(10);
    base2.insert(20);
    set s2(base2);
    s2.insert(10);  // duplicate
    s2.insert(30);
    CHECK(s2.has(10) == true, "set tree has 10");
    CHECK(s2.has(20) == true, "set tree has 20");
    CHECK(s2.has(30) == true, "set tree has 30");

    // set insert array (dedup)
    searchable_array_bag base3;
    int arr2[] = {1, 1, 2, 2};
    base3.insert(arr2, 4);
    set s3(base3);
    int arr3[] = {1, 2, 3};
    s3.insert(arr3, 3);
    CHECK(s3.has(1) == true, "set insert array has 1");
    CHECK(s3.has(2) == true, "set insert array has 2");
    CHECK(s3.has(3) == true, "set insert array has 3");

    // clear
    abag.clear();
    CHECK(abag.has(5) == false, "array_bag clear");
    CHECK(abag.empty() == true, "array_bag empty after clear");
    tbag.clear();
    CHECK(tbag.has(5) == false, "tree_bag clear");
    CHECK(tbag.empty() == true, "tree_bag empty after clear");

    // copy constructor
    searchable_array_bag orig;
    orig.insert(42);
    searchable_array_bag copy(orig);
    CHECK(copy.has(42) == true, "array_bag copy has");
    copy.insert(100);
    CHECK(orig.has(100) == false, "array_bag copy independent");

    // set get_bag
    searchable_array_bag base4;
    base4.insert(99);
    set s4(base4);
    CHECK(s4.get_bag().has(99) == true, "set get_bag has");

    // empty bag edge cases
    searchable_array_bag ebag;
    CHECK(ebag.has(0) == false, "empty array_bag has returns false");
    CHECK(ebag.count() == 0, "empty array_bag count 0");
    CHECK(ebag.empty() == true, "empty array_bag empty");
    searchable_tree_bag etbag;
    CHECK(etbag.has(0) == false, "empty tree_bag has returns false");
    CHECK(etbag.empty() == true, "empty tree_bag empty");

    // set with empty bag
    set sempty(ebag);
    CHECK(sempty.has(1) == false, "set empty has false");

    // duplicate inserts (same value multiple times)
    searchable_array_bag dup;
    dup.insert(5);
    dup.insert(5);
    dup.insert(5);
    CHECK(dup.has(5) == true, "array_bag duplicate insert has");
    CHECK(dup.count() == 3, "array_bag duplicate insert count");

    // tree bag duplicate insert
    searchable_tree_bag tdupe;
    tdupe.insert(7);
    tdupe.insert(7);
    CHECK(tdupe.has(7) == true, "tree_bag duplicate insert has");

    // set dedup: insert same value into set multiple times
    searchable_array_bag sbase;
    int sarr[] = {5, 5, 5};
    sbase.insert(sarr, 3);
    set sdedup(sbase);
    CHECK(sdedup.has(5) == true, "set dedup has 5");

    // large set (many elements)
    searchable_array_bag lbag;
    for (int i = 0; i < 100; i++)
        lbag.insert(i);
    CHECK(lbag.count() == 100, "large array_bag count");
    CHECK(lbag.has(50) == true, "large array_bag has 50");
    CHECK(lbag.has(101) == false, "large array_bag has 101 false");

    // tree bag large set
    searchable_tree_bag ltbag;
    for (int i = 0; i < 100; i++)
        ltbag.insert(i);
    CHECK(ltbag.has(99) == true, "large tree_bag has 99");
    CHECK(ltbag.has(100) == false, "large tree_bag has 100 false");

    std::cout << "\npolyset: " << pass << " passed, " << fail << " failed" << std::endl;
    return fail == 0 ? 0 : 1;
}
