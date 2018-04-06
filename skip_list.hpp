#pragma once

#include <cstddef>
#include <cstdlib>
#include <algorithm>
#include <iterator>

template <class T, class Compare = std::less<T>, std::size_t MAX_LEVEL = 6>
class skip_list {
public:
    struct node {
        using value_type = T;
        value_type value;
        node* forward[];
    };
    using node_type = node;
    class iterator : public std::iterator<std::forward_iterator_tag, node> {
    public:
        explicit iterator(node_type* node = nullptr) : _cur{node} {}

        iterator& operator++() {
            if (_cur != nullptr) {
                _cur = _cur->forward[0];
            }
            return *this;
        }

        iterator operator++(int) {
            iterator tmp(_cur);
            ++*this;
            return tmp;
        }

        bool operator==(const iterator& rhs) {
            if (this == &rhs) {
                return true;
            }
            return this->_cur == rhs._cur;
        }

        bool operator!=(const iterator& rhs) {
            return !(*this == rhs);
        }

        typename node::value_type& operator*() {
            return _cur->value;
        }

        typename node::value_type* operator->() {
            return &_cur->value;
        }

    private:
        node_type* _cur;
    };
    using value_type = T;
    using difference_type = typename iterator::difference_type;
    using pointer = typename iterator::pointer;
    using reference = typename iterator::reference;
    using iterator_category = typename iterator::iterator_category;

    skip_list();

    ~skip_list();

    void insert(value_type value);

    int erase(value_type value);

    node_type* find(value_type value);

    bool empty() const {
        return size() == 0;
    }

    std::size_t size() const {
        return _size;
    }

    iterator begin() {
        if (empty()) {
            return iterator(nullptr);
        }
        return iterator(_header->forward[0]);
    }

    iterator end() {
        return iterator(nullptr);
    }

private:
    std::size_t generate_node_level(double p, std::size_t maxLevel) {
        std::size_t level = 1;
        while (drand48() < p)
            level++;
        return (level > maxLevel) ? maxLevel : level;
    }

    bool equal(const value_type &lh, const value_type &rh) {
        return !_compare(lh, rh) && !_compare(rh, lh);
    }

    node_type* createNode(std::size_t level, value_type value) {
        node_type* node = static_cast<node_type *>(malloc(sizeof(node_type) + level * sizeof(node_type*)));
        node->value = value;
    }

    void deleteNode(node_type* node) {
        free(node);
    }

private:
    node_type* _header;
    Compare _compare;
    std::size_t _level;
    std::size_t _size;
};

template <class T, class Compare, std::size_t MAX_LEVEL>
skip_list<T, Compare, MAX_LEVEL>::skip_list()
        : _header{nullptr},
          _level{0},
          _size{0} {
    _header = static_cast<node_type*>(malloc(sizeof(node_type) + MAX_LEVEL * sizeof(node_type*)));
    for (std::size_t i = 0; i < MAX_LEVEL; ++i) {
        _header->forward[i] = nullptr;
    }
}

template <class T, class Compare, std::size_t MAX_LEVEL>
skip_list<T, Compare, MAX_LEVEL>::~skip_list() {

}

template <class T, class Compare, std::size_t MAX_LEVEL>
void skip_list<T, Compare, MAX_LEVEL>::insert(value_type value) {
    node_type* update[MAX_LEVEL] = {nullptr};
    node_type *p = _header, *q = nullptr;
    for (int i = _level - 1; i >= 0; --i) {
        while ((q = p->forward[i]) != nullptr && _compare(q->value, value))
            p = q;
        update[i] = p;
    }
    // 相等
    if (q != nullptr && equal(q->value, value)) {
        return;
    } else {
        std::size_t level = generate_node_level(0.5f, MAX_LEVEL);
        if (level > _level) {
            for (std::size_t i = _level; i < level; ++i) {
                update[i] = _header;
            }
            _level = level;
        }
        auto* new_node = createNode(level, value);
        for (std::size_t i = 0; i < level; ++i) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }
        ++_size;
    }
    return;
}

template <class T, class Compare, std::size_t MAX_LEVEL>
int skip_list<T, Compare, MAX_LEVEL>::erase(value_type value) {
    node_type* update[MAX_LEVEL];
    node_type *p = _header, *q = nullptr;
    for (int i = _level - 1; i >= 0; --i) {
        while ((q = p->forward[i]) != nullptr && _compare(q->value, value))
            p = q;
        update[i] = p;
    }
    if (q != nullptr && equal(q->value, value)) {
        for (std::size_t i = 0; i < _level; ++i) {
            if (update[i]->forward[i] == q)
                update[i]->forward[i] = q->forward[i];
        }
        deleteNode(q);
        for (int j = _level - 1; j >= 0; --j) {
            if (_header->forward[j] == nullptr) {
                --_level;
            }
        }
        --_size;
        return 1;
    }
    return 0;
}

template <class T, class Compare, std::size_t MAX_LEVEL>
typename skip_list<T, Compare, MAX_LEVEL>::node_type* skip_list<T, Compare, MAX_LEVEL>::find(value_type value) {
    node_type *p = _header, *q = nullptr;
    for (int i = _level - 1; i >= 0; i--) {
        while ((q = p->forward[i]) != nullptr && _compare(q->value, value))
            p = q;
    }
    if (q != nullptr && equal(q->value, value)) {
        return q;
    } else {
        return nullptr;
    }
}