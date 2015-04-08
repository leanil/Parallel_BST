#include <memory>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <algorithm>

template <class K, class V, class Compare = std::less<K>, class Distance = std::minus<K>>
class BST
{
private:

	const int core_num = 8;

	struct Node
	{
		Node(std::pair<K, V> val, typename BST::Node_ptr parent, bool red = true)
			: val{ std::move(val) }, red{ red }, parent{ std::move(parent) } {}
		K& key() { return val.first; }
		V& value() { return val.second; }
		std::pair<K, V> val;
		bool red;
		typename BST::Node_ptr left_child, right_child;
		std::weak_ptr<Node> parent;
	};

	using Node_ptr = std::shared_ptr < Node > ;
	using child_selector = std::function < Node_ptr&(Node_ptr) > ;

public:

	class BST_const_iterator;

	typedef std::pair<K, V> value_type;
	typedef	BST_const_iterator iterator;
	typedef iterator const_iterator;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef int difference_type;
	typedef unsigned size_type;

	class BST_const_iterator
	{
	public:

		typedef std::bidirectional_iterator_tag iterator_category;
		typedef typename BST::value_type value_type;
		typedef typename BST::difference_type difference_type;
		typedef typename BST::pointer pointer;
		typedef typename BST::reference reference;

		BST_const_iterator() {}
		BST_const_iterator(Node_ptr ptr) :ptr{ ptr } {}
		BST_const_iterator(const BST_const_iterator&) = default;
		BST_const_iterator(BST_const_iterator&& other) :ptr{ std::move(other.ptr) } {}

		value_type operator*() {
			return ptr->val;
		}

		const pointer operator->() {
			return &ptr->val;
		}

		BST_const_iterator& operator++() {
			if (!ptr);
			else if (ptr->right_child) {
				for (ptr = ptr->right_child; ptr->left_child; ptr = ptr->left_child);
			}
			else {
				for (Node_ptr par; (par = parent(ptr)) && ptr == par->right_child; ptr = par);
				ptr = parent(ptr);
			}
			return *this;
		}

		BST_const_iterator& operator++(int) {
			BST_const_iterator tmp = *this;
			++*this;
			return tmp;
		}

		BST_const_iterator& operator--() {
			if (!ptr) {
				for (ptr = root; ptr->right_child; ptr = ptr->right_child);
			}
			else if (ptr->left_child) {
				for (ptr = ptr->left_child; ptr->right_child; ptr = ptr->right_child);
			}
			else {
				for (Node_ptr par; par = parent(ptr) && ptr == par->left_child; ptr = par);
				ptr = parent(ptr);
			}
			return *this
		}

		BST_const_iterator& operator--(int) {
			BST_const_iterator tmp = *this;
			--*this;
			return tmp;
		}

		bool operator==(const BST_const_iterator& other) {
			return ptr == other.ptr;
		}

		bool operator!=(const BST_const_iterator& other) {
			return !(*this == other);
		}

	private:
		Node_ptr ptr;
	};

	BST(K lb, K ub) : lw_bound{ std::move(lb) }, up_bound{ std::move(ub) }, act_size{ 0 } {}

	BST(const BST& other) :lw_bound{ other.lw_bound }, up_bound{ other.up_bound }, act_size{ other.act_size } {
		deep_copy(root, other.root, Node_ptr());
	}

	BST(BST&& other) : lw_bound{ std::move(other.lw_bound) }, up_bound{ std::move(other.up_bound) },
		act_size{ other.act_size }, root{ std::move(other.root) } {}

	template<class InputIt>
	BST(K lb, K ub, InputIt first, InputIt last) : BST(lb, ub) {
		std::for_each(first, last, [=] (const_reference val) {insert(val); });
	}

	BST& operator=(BST other) {
		swap(other);
		return *this;
	}

	iterator begin() {
		Node_ptr tmp;
		for (tmp = root; tmp->left_child; tmp = tmp->left_child);
		return iterator{ tmp };
	}

	iterator end() {
		return iterator();
	}

	size_type size() {
		return act_size;
	}

	bool empty() {
		return act_size == 0;
	}

	void insert(std::pair<K, V> val) {
		if (Compare()(val.first, lw_bound) || !Compare()(val.first, up_bound)) { return; }
		act_size++;
		if (!root) {
			root = std::make_shared<Node>(std::move(val), Node_ptr());
			root->red = false;
			return;
		}
		Node_ptr act, child{ root };
		while (act = child, child = choose_child(act, val.first));
		act = choose_child(act, val.first) = std::make_shared<Node>(std::move(val), act);

		auto left_child = [] (const Node_ptr& parent) ->Node_ptr& {return parent->left_child; };
		auto right_child = [] (const Node_ptr& parent) ->Node_ptr& {return parent->right_child; };

		while (parent(act) && parent(act)->red) {
			if (parent(act) == grand_parent(act)->left_child) { //2. körben nincs nagyapja
				restore(act, left_child, right_child);
			}
			else {
				restore(act, right_child, left_child);
			}
		}
		root->red = false;
	}

	iterator find_nearest(const K& key) {
		if (empty()) {
			throw logic_error("find_nearest in empty tree");
		}
		Node_ptr less{ root }, more{ root }, it{ root };
		while (it && it->key() != key) {
			(Compare()(key, it->key()) ? more : less) = it;
			it = choose_child(it, key);
		}
		if (it) { return it; }
		return iterator(Compare()(Distance()(key, less->key()), Distance()(more->key(), key)) ? less : more);
	}

	template<class Range>
	std::vector<iterator> find_nearest(const Range& rng) {
		return find_nearest(rng.begin(), rng.end());
	}

	template<class InputIt>
	std::vector<iterator> find_nearest(InputIt first, InputIt last) {
		if (empty()) {
			throw logic_error("find_nearest in empty tree");
		}
		auto size = std::distance(first, last);
		std::vector<iterator> result(size);
		std::vector<std::thread> threads;

		for (int i = 0; i < core_num - 1; i++) {
			auto from = first;
			std::advance(first, size / core_num);
			threads.push_back(std::thread{
				[=, &result]() mutable {std::transform(from, first, result.begin() + i*(size / core_num),
				[=] (const K& key) {return find_nearest(key); }); } });
		}
		threads.push_back(std::thread{
			[=, &result]() mutable {std::transform(first, last, result.begin() + (core_num - 1)*(size / core_num),
			[=] (const K& key) {return find_nearest(key); }); } });

			for (auto& t : threads) { t.join(); }
			return result;
	}

	BST& change_bounds(K lb, K ub) {
		bool shorter = Compare()(lw_bound, lb) || Compare()(ub, up_bound);
		if (shorter) {
			return *this = BST(lb, ub, begin(), end());
		}
		lw_bound = lb, up_bound = ub;
		return *this;
	}

	void swap(BST& other) {
		std::swap(lw_bound, other.lw_bound);
		std::swap(up_bound, other.up_bound);
		std::swap(root, other.root);
		std::swap(act_size, other.act_size);
	}

	template<class UnaryFunction>
	void recursive_for_each(UnaryFunction f) {
		recursive_for_each(root, f);
	}

	template <class K, class V>
	friend std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst);


private:

	static Node_ptr& choose_child(const Node_ptr& parent, const K& key) {
		return Compare()(key, parent->key()) ? parent->left_child : parent->right_child;
	}

	static Node_ptr parent(const Node_ptr& child) {
		return child ? child->parent.lock() : child;
	}

	static Node_ptr grand_parent(const Node_ptr& child) {
		return parent(child) ? parent(child)->parent.lock() : parent(child);
	}

	template<class UnaryFunction>
	void recursive_for_each(Node_ptr root, UnaryFunction f) {
		if (!root) { return; }
		recursive_for_each(root->left_child, f);
		f(root->val);
		recursive_for_each(root->right_child, f);
	}

	void deep_copy(Node_ptr& target, std::shared_ptr<const Node> source, Node_ptr parent) {
		if (!source) { return; }
		target = std::make_shared<Node>(source->val, std::move(parent), source->red);
		deep_copy(target->left_child, source->left_child, target);
		deep_copy(target->right_child, source->right_child, target);
	}

	void restore(Node_ptr& child, const child_selector& side1_child, const child_selector& side2_child) {
		auto uncle = side2_child(grand_parent(child));
		if (uncle && uncle->red) {
			parent(child)->red = false;
			uncle->red = false;
			grand_parent(child)->red = true;
			child = grand_parent(child);
		}
		else {
			if (child == side2_child(parent(child))) {
				child = parent(child);
				rotate(child, side1_child, side2_child);
			}
			parent(child)->red = false;
			grand_parent(child)->red = true;
			rotate(grand_parent(child), side2_child, side1_child);
		}
	}

	void rotate(Node_ptr x, const child_selector& side1_child, const child_selector& side2_child) {
		auto y = side2_child(x);
		side2_child(x) = side1_child(y);
		if (side1_child(y)) {
			side1_child(y)->parent = x;
		}
		y->parent = x->parent;
		(x->parent.lock() ?
			(x == side1_child(parent(x)) ? side1_child(parent(x)) : side2_child(parent(x))) :
			root) = y;
		side1_child(y) = x;
		x->parent = y;
	}

	K lw_bound, up_bound;
	Node_ptr root;
	size_type act_size;
};

template <class K, class V>
std::ostream& operator<<(std::ostream& out, BST<K, V>& bst) {
	bst.recursive_for_each([&] (std::pair<K, V> val) { out << "{" << val.first << "," << val.second << "} "; });
	return out;
}