/*
 * tree_bag.cpp — 42 Exam Rank 05 (level 0), polyset given-code
 */
#include "tree_bag.hpp"

#include <cstddef>

tree_bag::tree_bag() : m_root(NULL) {}

tree_bag::tree_bag(const tree_bag &other) : m_root(NULL)
{
    m_root = copy_tree(other.m_root);
}

tree_bag &tree_bag::operator=(const tree_bag &other)
{
    if (this != &other)
    {
        free_tree(m_root);
        m_root = NULL;
        m_root = copy_tree(other.m_root);
    }
    return *this;
}

tree_bag::~tree_bag()
{
    free_tree(m_root);
}

t_node *tree_bag::copy_tree(t_node *node) const
{
    if (node == NULL)
        return NULL;
    t_node *copy = new t_node;
    copy->value = node->value;
    copy->left = copy_tree(node->left);
    copy->right = copy_tree(node->right);
    return copy;
}

void tree_bag::free_tree(t_node *node)
{
    if (node == NULL)
        return;
    free_tree(node->left);
    free_tree(node->right);
    delete node;
}

void tree_bag::insert_node(t_node *&node, int value)
{
    if (node == NULL)
    {
        node = new t_node;
        node->value = value;
        node->left = NULL;
        node->right = NULL;
        return;
    }
    if (value < node->value)
        insert_node(node->left, value);
    else
        insert_node(node->right, value);
}

void tree_bag::insert(int value)
{
    insert_node(m_root, value);
}

void tree_bag::insert(int *arr, int n)
{
    for (int i = 0; i < n; i++)
        insert(arr[i]);
}

int tree_bag::count_node(t_node *node) const
{
    if (node == NULL)
        return 0;
    return 1 + count_node(node->left) + count_node(node->right);
}

bool tree_bag::has_node(t_node *node, int value) const
{
    if (node == NULL)
        return false;
    if (value == node->value)
        return true;
    if (value < node->value)
        return has_node(node->left, value);
    return has_node(node->right, value);
}

int tree_bag::count() const
{
    return count_node(m_root);
}

bool tree_bag::empty() const
{
    return m_root == NULL;
}

void tree_bag::clear()
{
    free_tree(m_root);
    m_root = NULL;
}
