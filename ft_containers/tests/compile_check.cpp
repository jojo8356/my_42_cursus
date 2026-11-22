#include "vector.hpp"
#include "list.hpp"
#include "deque.hpp"
#include "stack.hpp"
#include "queue.hpp"
#include "map.hpp"
#include "set.hpp"
#include "multimap.hpp"
#include "multiset.hpp"
#include <iostream>

int main()
{
    ft::vector<int> v;
    for (int i = 0; i < 10; ++i) v.push_back(i);
    ft::vector<int>::iterator it = v.begin();
    std::cout << *it << v[5] << v.back() << v.size() << "\n";

    ft::map<int, std::string> m;
    m[1] = "one";
    m.insert(ft::make_pair(2, std::string("two")));
    std::cout << m[1] << m[2] << m.size() << "\n";

    ft::stack<int> s;
    s.push(1); s.push(2);
    std::cout << s.top() << s.size() << "\n";

    ft::set<int> st;
    st.insert(3); st.insert(1); st.insert(2);
    std::cout << *st.begin() << *st.rbegin() << st.size() << "\n";

    ft::list<int> l;
    l.push_back(1); l.push_front(0);
    std::cout << l.front() << l.back() << "\n";

    ft::deque<int> d;
    d.push_back(1); d.push_front(0);
    std::cout << d[0] << d[1] << "\n";

    ft::queue<int> q;
    q.push(1); q.push(2);
    std::cout << q.front() << q.back() << "\n";

    ft::multimap<int, std::string> mm;
    mm.insert(ft::make_pair(1, "a"));
    mm.insert(ft::make_pair(1, "b"));
    std::cout << mm.count(1) << mm.size() << "\n";

    ft::multiset<int> ms;
    ms.insert(1); ms.insert(1); ms.insert(2);
    std::cout << ms.count(1) << ms.size() << "\n";

    return 0;
}
