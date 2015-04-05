#include <memory>
#include <utility>
#include <stdexcept>
#include <iostream>

template <class K, class V>
class BST
{
public:

	struct Node
	{
		Node(const K& key, const V& value) : key{ key }, value{ value } {}
		Node(const K& key, const V& value, const std::shared_ptr<Node>& parent) : key{ key }, value{ value }, parent{ parent } {}
		K key;
		V value;
		std::shared_ptr<Node> left_child, right_child;
		std::weak_ptr<Node> parent;
	};

	BST(const K& lb, const K& ub) : lw_bound{ lb }, up_bound{ ub } {}

	void insert(const std::pair<K, V>& val) {
		if (val.first < lw_bound || val.first >= up_bound) {
			throw std::out_of_range("key is out of boundaries");
		}
		if (!root) {
			root = std::make_shared<Node>(val.first, val.second);
			return;
		}
		std::shared_ptr<Node> parent, child;
		for (child = root; parent = child, child = (val.first < parent->key ? parent->left_child : parent->right_child););
		(val.first < parent->key ? parent->left_child : parent->right_child) = std::make_shared<Node>(val.first, val.second, parent);

	}

	std::shared_ptr<Node> find_nearest(const K& key) {
		if (empty()) {
			throw logic_error("find_nearest in empty tree");
		}
		std::shared_ptr<Node> less{ root }, more{ root }, it{ root };
		while (it && it->key != key) {
			(key < it->key ? more : less) = it;
			it = (key < it->key ? it->left_child : it->right_child);
		}
		if (it) { return it; }
		return key - less->key < more->key - key ? less : more;
	}

	bool empty() {
		return !root;
	}

	template <class K, class V>
	friend std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst);

private:

	void traverse(const std::shared_ptr<Node>& root, std::ostream& out) const {
		if (!root) { return; }
		traverse(root->left_child, out);
		out << root->value << " ";
		traverse(root->right_child, out);
	}

	K lw_bound, up_bound;
	std::shared_ptr<Node> root;
};

template <class K, class V>
std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst) {
	bst.traverse(bst.root, out);
	return out;
}