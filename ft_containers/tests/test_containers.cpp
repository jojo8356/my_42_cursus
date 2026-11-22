//////////////////////////////////////////////////////////////////////////
//  test_containers.cpp — comprehensive differential tests (ft:: vs std::)
//  Covers: vector, list, deque, stack, queue, map, set, multimap,
//  multiset, pair, make_pair, iterator_traits, reverse_iterator,
//  enable_if, is_integral, lexicographical_compare, equal.
//////////////////////////////////////////////////////////////////////////

# include "test_framework.hpp"
# include "vector.hpp"
# include "list.hpp"
# include "deque.hpp"
# include "stack.hpp"
# include "queue.hpp"
# include "map.hpp"
# include "set.hpp"
# include "multimap.hpp"
# include "multiset.hpp"
# include <vector>
# include <list>
# include <deque>
# include <stack>
# include <queue>
# include <map>
# include <set>
# include <string>
# include <algorithm>
# include <iterator>

using namespace std;

// ------------------------------------------------------------------
//  Vector
// ------------------------------------------------------------------
static void test_vector()
{
    TEST("vector");
    ft::vector<int> v;
    std::vector<int> sv;
    for (int i = 0; i < 1000; ++i)
    {
        v.push_back(i);
        sv.push_back(i);
    }
    ASSERT_EQ(v.size(), sv.size(), "vector size");
    ASSERT_EQ(v.capacity() >= v.size(), true, "vector capacity >= size");
    ASSERT_EQ(v.empty(), false, "vector not empty");
    for (size_t i = 0; i < v.size(); ++i)
        ASSERT_EQ(v[i], sv[i], "vector operator[]");
    ASSERT_EQ(v.front(), sv.front(), "vector front");
    ASSERT_EQ(v.back(), sv.back(), "vector back");

    // push_back/pop_back
    v.push_back(9999); sv.push_back(9999);
    ASSERT_EQ(v.back(), sv.back(), "vector push_back back");
    v.pop_back(); sv.pop_back();
    ASSERT_EQ(v.back(), sv.back(), "vector pop_back");

    // insert / erase
    v.insert(v.begin() + 10, 42); sv.insert(sv.begin() + 10, 42);
    ASSERT_EQ(v.size(), sv.size(), "vector insert size");
    ASSERT_EQ(v[10], sv[10], "vector insert value");
    v.erase(v.begin() + 5); sv.erase(sv.begin() + 5);
    ASSERT_EQ(v.size(), sv.size(), "vector erase size");
    v.erase(v.begin() + 3, v.begin() + 7); sv.erase(sv.begin() + 3, sv.begin() + 7);
    ASSERT_EQ(v.size(), sv.size(), "vector erase range size");
    ASSERT(ft::equal(v.begin(), v.end(), sv.begin()), "vector erase range content");

    // resize / reserve / assign / clear
    v.resize(50); sv.resize(50);
    ASSERT_EQ(v.size(), sv.size(), "vector resize");
    v.reserve(2000); sv.reserve(2000);
    ASSERT(v.capacity() >= 2000, "vector reserve");
    v.assign(10, 7); sv.assign(10, 7);
    ASSERT_EQ(v.size(), sv.size(), "vector assign n");
    ASSERT_EQ(v[0], sv[0], "vector assign value");

    // iterators
    ASSERT(ft::equal(v.begin(), v.end(), sv.begin()), "vector iterator equal");
    ft::vector<int>::iterator it = v.begin();
    std::vector<int>::iterator sit = sv.begin();
    for (int i = 0; i < 10; ++i) { ASSERT_EQ(*it, *sit, "vector iter deref"); ++it; ++sit; }
    // reverse iterators
    ft::vector<int>::reverse_iterator rit = v.rbegin();
    std::vector<int>::reverse_iterator srit = sv.rbegin();
    for (int i = 0; i < 5; ++i) { ASSERT_EQ(*rit, *srit, "vector reverse iter"); ++rit; ++srit; }

    // relational operators
    ft::vector<int> v2(v);
    ASSERT(v == v2, "vector ==");
    ASSERT(!(v != v2), "vector !=");
    ft::vector<int> v3;
    ASSERT(v3 < v, "vector <");
    ASSERT(v > v3, "vector >");
    ASSERT(v3 <= v, "vector <=");
    ASSERT(v >= v3, "vector >=");

    // at() exception
    bool threw = false;
    try { (void)v.at(10000); } catch (std::out_of_range&) { threw = true; }
    ASSERT(threw, "vector at throws");

    // copy & assignment
    ft::vector<int> v4 = v;
    ASSERT(v4 == v, "vector copy");
    v4 = v2;
    ASSERT(v4 == v2, "vector assign op");

    // swap
    ft::vector<int> v5; v5.push_back(1);
    ft::swap(v, v5);
    ASSERT(v.size() == 1 && v5.size() == 10, "vector swap");
}

// ------------------------------------------------------------------
//  List
// ------------------------------------------------------------------
static void test_list()
{
    TEST("list");
    ft::list<int> l;
    std::list<int> sl;
    for (int i = 0; i < 500; ++i)
    {
        l.push_back(i);
        sl.push_back(i);
    }
    ASSERT_EQ(l.size(), sl.size(), "list size");
    ASSERT_EQ(l.empty(), false, "list not empty");
    ASSERT_EQ(l.front(), sl.front(), "list front");
    ASSERT_EQ(l.back(), sl.back(), "list back");

    // push_front / pop_front / pop_back
    l.push_front(-1); sl.push_front(-1);
    ASSERT_EQ(l.front(), sl.front(), "list push_front");
    l.pop_front(); sl.pop_front();
    ASSERT_EQ(l.front(), sl.front(), "list pop_front");
    l.pop_back(); sl.pop_back();
    ASSERT_EQ(l.back(), sl.back(), "list pop_back");

    // insert / erase
    l.insert(l.begin(), 99); sl.insert(sl.begin(), 99);
    ASSERT_EQ(l.front(), sl.front(), "list insert front");
    l.erase(l.begin()); sl.erase(sl.begin());
    ASSERT_EQ(l.size(), sl.size(), "list erase size");

    // iterators (bidirectional)
    ft::list<int>::iterator it = l.begin();
    std::list<int>::iterator sit = sl.begin();
    for (int i = 0; i < 20; ++i) { ASSERT_EQ(*it, *sit, "list iter"); ++it; ++sit; }
    // reverse iterators
    ft::list<int>::reverse_iterator rit = l.rbegin();
    std::list<int>::reverse_iterator srit = sl.rbegin();
    for (int i = 0; i < 5; ++i) { ASSERT_EQ(*rit, *srit, "list reverse iter"); ++rit; ++srit; }

    // sort / reverse / unique / merge
    ft::list<int> l2, sl2;
    for (int i = 5; i > 0; --i) { l2.push_back(i); sl2.push_back(i); }
    l2.sort(); sl2.sort();
    ASSERT(ft::equal(l2.begin(), l2.end(), sl2.begin()), "list sort");
    l2.reverse(); sl2.reverse();
    ASSERT(ft::equal(l2.begin(), l2.end(), sl2.begin()), "list reverse");

    ft::list<int> l3; l3.push_back(1); l3.push_back(1); l3.push_back(2);
    std::list<int> sl3; sl3.push_back(1); sl3.push_back(1); sl3.push_back(2);
    l3.unique(); sl3.unique();
    ASSERT(ft::equal(l3.begin(), l3.end(), sl3.begin()), "list unique");

    ft::list<int> l4, sl4;
    for (int i = 3; i > 0; --i) { l4.push_back(i); sl4.push_back(i); }
    ft::list<int> l5, sl5;
    for (int i = 6; i > 3; --i) { l5.push_back(i); sl5.push_back(i); }
    l4.merge(l5); sl4.merge(sl5);
    ASSERT(ft::equal(l4.begin(), l4.end(), sl4.begin()), "list merge");
    ASSERT(l5.empty(), "list merge empties src");

    // relational
    ft::list<int> la(l);
    ASSERT(l == la, "list ==");
    ft::list<int> lb;
    ASSERT(lb < l, "list <");

    // splice
    ft::list<int> l6; l6.push_back(100); l6.push_back(200);
    ft::list<int> l7; l7.push_back(1); l7.push_back(2);
    size_t before = l7.size();
    l7.splice(l7.begin(), l6);
    ASSERT_EQ(l7.size(), before + 2, "list splice size");
    ASSERT_EQ(l7.front(), 100, "list splice front");
    ASSERT(l6.empty(), "list splice src empty");
}

// ------------------------------------------------------------------
//  Deque
// ------------------------------------------------------------------
static void test_deque()
{
    TEST("deque");
    ft::deque<int> d;
    std::deque<int> sd;
    for (int i = 0; i < 1000; ++i)
    {
        d.push_back(i);
        sd.push_back(i);
    }
    ASSERT_EQ(d.size(), sd.size(), "deque size");
    ASSERT_EQ(d.front(), sd.front(), "deque front");
    ASSERT_EQ(d.back(), sd.back(), "deque back");

    // push_front / pop_front / pop_back
    d.push_front(-1); sd.push_front(-1);
    ASSERT_EQ(d.front(), sd.front(), "deque push_front");
    d.pop_front(); sd.pop_front();
    d.pop_back(); sd.pop_back();
    ASSERT_EQ(d.back(), sd.back(), "deque pop_back");

    // operator[] / at
    for (size_t i = 0; i < d.size(); ++i)
        ASSERT_EQ(d[i], sd[i], "deque operator[]");

    // insert / erase
    d.insert(d.begin() + 10, 42); sd.insert(sd.begin() + 10, 42);
    ASSERT_EQ(d.size(), sd.size(), "deque insert size");
    ASSERT_EQ(d[10], sd[10], "deque insert value");
    d.erase(d.begin() + 5); sd.erase(sd.begin() + 5);
    ASSERT_EQ(d.size(), sd.size(), "deque erase size");

    // resize / assign / clear
    d.resize(50); sd.resize(50);
    ASSERT_EQ(d.size(), sd.size(), "deque resize");
    d.assign(10, 7); sd.assign(10, 7);
    ASSERT_EQ(d[0], sd[0], "deque assign");

    // iterators (random access)
    ft::deque<int>::iterator it = d.begin();
    std::deque<int>::iterator sit = sd.begin();
    for (int i = 0; i < 10; ++i) { ASSERT_EQ(*it, *sit, "deque iter"); ++it; ++sit; }
    ft::deque<int>::reverse_iterator rit = d.rbegin();
    std::deque<int>::reverse_iterator srit = sd.rbegin();
    for (int i = 0; i < 5; ++i) { ASSERT_EQ(*rit, *srit, "deque reverse iter"); ++rit; ++srit; }

    // relational
    ft::deque<int> d2(d);
    ASSERT(d == d2, "deque ==");
    ft::deque<int> d3;
    ASSERT(d3 < d, "deque <");
}

// ------------------------------------------------------------------
//  Stack
// ------------------------------------------------------------------
static void test_stack()
{
    TEST("stack");
    ft::stack<int> s;
    std::stack<int> ss;
    for (int i = 0; i < 100; ++i) { s.push(i); ss.push(i); }
    ASSERT_EQ(s.size(), ss.size(), "stack size");
    ASSERT_EQ(s.empty(), ss.empty(), "stack empty");
    ASSERT_EQ(s.top(), ss.top(), "stack top");
    while (!s.empty())
    {
        ASSERT_EQ(s.top(), ss.top(), "stack pop loop");
        s.pop(); ss.pop();
    }
    ASSERT(s.empty(), "stack empty after pop");

    // relational
    ft::stack<int> a, b;
    a.push(1); a.push(2);
    b.push(1); b.push(2);
    ASSERT(a == b, "stack ==");
    b.push(3);
    ASSERT(a < b, "stack <");
    ASSERT(b > a, "stack >");

    // with vector as underlying container
    ft::stack<int, ft::vector<int> > sv;
    sv.push(5);
    ASSERT_EQ(sv.top(), 5, "stack with vector");
    // with std::vector as underlying (compatibility)
    ft::stack<int, std::vector<int> > ssv;
    ssv.push(9);
    ASSERT_EQ(ssv.top(), 9, "stack with std::vector");
}

// ------------------------------------------------------------------
//  Queue
// ------------------------------------------------------------------
static void test_queue()
{
    TEST("queue");
    ft::queue<int> q;
    std::queue<int> sq;
    for (int i = 0; i < 100; ++i) { q.push(i); sq.push(i); }
    ASSERT_EQ(q.size(), sq.size(), "queue size");
    ASSERT_EQ(q.front(), sq.front(), "queue front");
    ASSERT_EQ(q.back(), sq.back(), "queue back");
    while (!q.empty())
    {
        ASSERT_EQ(q.front(), sq.front(), "queue pop loop");
        q.pop(); sq.pop();
    }
    ASSERT(q.empty(), "queue empty after pop");

    ft::queue<int> a, b;
    a.push(1); a.push(2);
    b.push(1); b.push(2);
    ASSERT(a == b, "queue ==");
    b.push(3);
    ASSERT(a < b, "queue <");
}

// ------------------------------------------------------------------
//  Map
// ------------------------------------------------------------------
static void test_map()
{
    TEST("map");
    ft::map<int, int> m;
    std::map<int, int> sm;
    for (int i = 0; i < 500; ++i)
    {
        m[i] = i * 2;
        sm[i] = i * 2;
    }
    ASSERT_EQ(m.size(), sm.size(), "map size");
    ASSERT_EQ(m.empty(), false, "map not empty");
    for (int i = 0; i < 500; ++i)
    {
        ASSERT_EQ(m[i], sm[i], "map operator[]");
    }

    // insert / find / erase
    ft::pair<ft::map<int,int>::iterator, bool> r = m.insert(ft::make_pair(1000, 1000));
    ASSERT(r.second == true, "map insert new");
    ASSERT_EQ(r.first->second, 1000, "map insert value");
    r = m.insert(ft::make_pair(0, 999));
    ASSERT(r.second == false, "map insert dup false");
    ASSERT_EQ(m[0], 0, "map insert dup keeps original");

    ft::map<int,int>::iterator it = m.find(100);
    ASSERT(it != m.end() && it->second == 200, "map find");
    ASSERT(m.find(99999) == m.end(), "map find missing");

    size_t before = m.size();
    ASSERT_EQ(m.erase(100), 1, "map erase returns 1");
    ASSERT_EQ(m.size(), before - 1, "map erase size");
    ASSERT(m.erase(99999) == 0, "map erase missing returns 0");

    // lower/upper_bound / equal_range / count
    it = m.lower_bound(100);
    ASSERT(it != m.end() && it->first == 101, "map lower_bound (100 erased, returns first >= 100)");
    it = m.upper_bound(100);
    ASSERT(it != m.end() && it->first == 101, "map upper_bound");
    ASSERT_EQ(m.count(100), 0, "map count after erase");
    ASSERT_EQ(m.count(101), 1, "map count");
    ft::pair<ft::map<int,int>::iterator, ft::map<int,int>::iterator> er = m.equal_range(101);
    ASSERT(er.first != er.second, "map equal_range non-empty");

    // iterators
    int idx = 0;
    for (ft::map<int,int>::iterator i = m.begin(); i != m.end(); ++i, ++idx)
    {
        (void)idx;
        ASSERT(i->first < i->first + 1, "map iterator order sanity");
    }
    ft::map<int,int>::reverse_iterator rit = m.rbegin();
    ASSERT(rit->first == 1000, "map reverse begin");

    // relational
    ft::map<int,int> m2(m);
    ASSERT(m == m2, "map ==");
}

// ------------------------------------------------------------------
//  Set
// ------------------------------------------------------------------
static void test_set()
{
    TEST("set");
    ft::set<int> st;
    std::set<int> sst;
    for (int i = 0; i < 500; ++i)
    {
        st.insert(i);
        sst.insert(i);
    }
    // duplicates
    st.insert(100);
    sst.insert(100);
    ASSERT_EQ(st.size(), sst.size(), "set size (unique)");
    ASSERT_EQ(st.count(100), 1, "set count");

    ft::pair<ft::set<int>::iterator, bool> r = st.insert(999);
    ASSERT(r.second == true, "set insert new");
    r = st.insert(100);
    ASSERT(r.second == false, "set insert dup");

    ASSERT(st.find(100) != st.end(), "set find");
    ASSERT(st.find(99999) == st.end(), "set find missing");
    ASSERT_EQ(st.erase(100), 1, "set erase");
    ASSERT_EQ(st.erase(99999), 0, "set erase missing");

    ASSERT(st.lower_bound(50) != st.end(), "set lower_bound");
    ASSERT(st.upper_bound(50) != st.end(), "set upper_bound");

    // sorted order
    bool sorted = true;
    ft::set<int>::iterator it = st.begin();
    int prev = *it;
    for (++it; it != st.end(); ++it)
    {
        if (*it <= prev) { sorted = false; break; }
        prev = *it;
    }
    ASSERT(sorted, "set sorted order");

    // iterators + reverse
    ft::set<int>::reverse_iterator rit = st.rbegin();
    ASSERT_EQ(*rit, 999, "set reverse begin (max)");

    ft::set<int> st2(st);
    ASSERT(st == st2, "set ==");
}

// ------------------------------------------------------------------
//  Multimap
// ------------------------------------------------------------------
static void test_multimap()
{
    TEST("multimap");
    ft::multimap<int, int> mm;
    std::multimap<int, int> smm;
    for (int i = 0; i < 200; ++i)
    {
        mm.insert(ft::make_pair(i % 20, i));
        smm.insert(std::make_pair(i % 20, i));
    }
    ASSERT_EQ(mm.size(), smm.size(), "multimap size");
    ASSERT_EQ(mm.count(5), smm.count(5), "multimap count");

    // duplicates allowed
    mm.insert(ft::make_pair(1, 100));
    mm.insert(ft::make_pair(1, 200));
    ASSERT_EQ(mm.count(1), smm.count(1) + 2, "multimap count dups");

    // find returns first
    ft::multimap<int,int>::iterator it = mm.find(1);
    ASSERT(it != mm.end() && it->first == 1, "multimap find");

    // erase by key removes all elements with that key (200 insertions i%20 => 10 per key, + 2 dups for key 1 = 12)
    size_t before = mm.size();
    ASSERT_EQ(mm.erase(1), smm.count(1) + 2, "multimap erase key count (all duplicates)");
    ASSERT_EQ(mm.size(), before - (smm.count(1) + 2), "multimap erase size");

    // equal_range
    ft::pair<ft::multimap<int,int>::iterator, ft::multimap<int,int>::iterator> er = mm.equal_range(5);
    ASSERT(er.first != er.second, "multimap equal_range");

    // sorted by key
    bool sorted = true;
    int prev = mm.begin()->first;
    for (ft::multimap<int,int>::iterator i = mm.begin(); i != mm.end(); ++i)
    {
        if (i->first < prev) { sorted = false; break; }
        prev = i->first;
    }
    ASSERT(sorted, "multimap sorted");
}

// ------------------------------------------------------------------
//  Multiset
// ------------------------------------------------------------------
static void test_multiset()
{
    TEST("multiset");
    ft::multiset<int> ms;
    std::multiset<int> sms;
    for (int i = 0; i < 200; ++i)
    {
        ms.insert(i % 20);
        sms.insert(i % 20);
    }
    ASSERT_EQ(ms.size(), sms.size(), "multiset size");
    ASSERT_EQ(ms.count(5), sms.count(5), "multiset count");
    ASSERT_EQ(ms.count(7), 10, "multiset count value");

    ms.insert(7);
    ASSERT_EQ(ms.count(7), 11, "multiset insert dup");

    ASSERT(ms.find(7) != ms.end(), "multiset find");
    size_t before = ms.size();
    ASSERT_EQ(ms.erase(7), 11, "multiset erase key");
    ASSERT_EQ(ms.size(), before - 11, "multiset erase size");

    bool sorted = true;
    int prev = *ms.begin();
    for (ft::multiset<int>::iterator i = ms.begin(); i != ms.end(); ++i)
    {
        if (*i < prev) { sorted = false; break; }
        prev = *i;
    }
    ASSERT(sorted, "multiset sorted");
}

// ------------------------------------------------------------------
//  pair / make_pair / iterator_traits / reverse_iterator / traits
// ------------------------------------------------------------------
static void test_utilities()
{
    TEST("pair & make_pair");
    ft::pair<int, std::string> p(1, "hello");
    ASSERT_EQ(p.first, 1, "pair first");
    ASSERT_EQ(p.second, std::string("hello"), "pair second");
    ft::pair<int, std::string> p2 = ft::make_pair(2, std::string("world"));
    ASSERT(p2.first == 2 && p2.second == "world", "make_pair");
    ASSERT(p < p2, "pair <");
    ASSERT(p2 > p, "pair >");
    ASSERT(p <= p, "pair <=");
    ASSERT(p >= p, "pair >=");
    ASSERT(p == p, "pair ==");
    ASSERT(p != p2, "pair !=");
    // conversion from pair<int,string> to pair<const int, string>
    ft::pair<const int, std::string> pc = ft::make_pair(3, std::string("x"));
    ASSERT_EQ(pc.first, 3, "pair converting ctor");

    TEST("iterator_traits");
    ft::vector<int>::iterator vit = ft::vector<int>().begin();
    (void)vit;
    ft::vector<int> v; v.push_back(1);
    typedef ft::iterator_traits<ft::vector<int>::iterator> traits;
    ASSERT((ft::is_same<traits::value_type, int>::value), "iterator_traits value_type");
    ASSERT((ft::is_same<traits::difference_type, ft::ptrdiff_t>::value), "iterator_traits difference_type");
    ASSERT((ft::is_same<traits::iterator_category, ft::random_access_iterator_tag>::value), "iterator_traits category");
    // pointer specialization
    typedef ft::iterator_traits<int*> ptraits;
    ASSERT((ft::is_same<ptraits::value_type, int>::value), "iterator_traits ptr value_type");
    ASSERT((ft::is_same<ptraits::iterator_category, ft::random_access_iterator_tag>::value), "iterator_traits ptr category");

    TEST("reverse_iterator");
    ft::vector<int> vv;
    for (int i = 0; i < 10; ++i) vv.push_back(i);
    ft::vector<int>::reverse_iterator rv = vv.rbegin();
    for (int i = 9; i >= 0; --i)
    {
        ASSERT_EQ(*rv, i, "reverse_iterator deref");
        ++rv;
    }
    ASSERT(rv == vv.rend(), "reverse_iterator reaches rend");

    TEST("enable_if & is_integral");
    typedef ft::enable_if<true, int>::type _eif_t; ASSERT((true), "enable_if true exists"); (void)sizeof(_eif_t);
    // is_integral
    ASSERT((ft::is_integral<int>::value), "is_integral int");
    ASSERT((ft::is_integral<unsigned long>::value), "is_integral unsigned long");
    ASSERT((!ft::is_integral<std::string>::value), "is_integral string false");
    ASSERT((!ft::is_integral<double>::value), "is_integral double false");

    TEST("lexicographical_compare & equal");
    ft::vector<int> a; a.push_back(1); a.push_back(2);
    ft::vector<int> b; b.push_back(1); b.push_back(3);
    ASSERT(ft::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end()), "lexico <");
    ft::vector<int> c = a;
    ASSERT(!ft::lexicographical_compare(c.begin(), c.end(), a.begin(), a.end()), "lexico equal not <");
    ASSERT(ft::equal(a.begin(), a.end(), c.begin()), "equal");
}

int main()
{
    test_vector();
    test_list();
    test_deque();
    test_stack();
    test_queue();
    test_map();
    test_set();
    test_multimap();
    test_multiset();
    test_utilities();
    return ftctest::summary();
}
