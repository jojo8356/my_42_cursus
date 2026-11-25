/*
 * tree_bag.hpp — 42 Exam Rank 05 (level 0), polyset given-code
 * Base bag storing elements in a binary search tree.
 */
#ifndef TREE_BAG_HPP
#define TREE_BAG_HPP

struct t_node
{
    int value;
    t_node *left;
    t_node *right;
};

class tree_bag
{
protected:
    t_node *m_root;

    t_node *copy_tree(t_node *node) const;
    void free_tree(t_node *node);
    void insert_node(t_node *&node, int value);
    int count_node(t_node *node) const;
    bool has_node(t_node *node, int value) const;

public:
    tree_bag();
    tree_bag(const tree_bag &other);
    tree_bag &operator=(const tree_bag &other);
    virtual ~tree_bag();

    void insert(int value);
    void insert(int *arr, int n);

    int count() const;
    bool empty() const;
    void clear();
};

#endif /* TREE_BAG_HPP */
