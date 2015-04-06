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
		Node(std::pair<K, V> val, std::shared_ptr<Node> parent = std::shared_ptr<Node>())
			: key{ std::move(val.first) }, value{ std::move(val.second) }, parent{ std::move(parent) } {}
		K key;
		V value;
		std::shared_ptr<Node> left_child, right_child;
		std::weak_ptr<Node> parent;
	};

	// Ineffective if K is not MoveConstructible.
	BST(K lb, K ub) : lw_bound{ std::move(lb) }, up_bound{ std::move(ub) } {}

	BST(const BST& other) :lw_bound{ other.lw_bound }, up_bound{ other.up_bound } {
		deep_copy(root, other.root, std::shared_ptr<Node>());
	}

	BST(BST&& other) : lw_bound{ std::move(other.lw_bound) }, up_bound{ std::move(other.up_bound) }, root{ std::move(other.root) } {}

	BST& operator=(BST other) {
		swap(other);
		return *this;
	}

	void swap(BST& other) {
		std::swap(lw_bound, other.lw_bound);
		std::swap(up_bound, other.up_bound);
		std::swap(root, other.root);
	}

	void insert(std::pair<K, V> val) {
		if (val.first < lw_bound || val.first >= up_bound) {
			throw std::out_of_range("key is out of boundaries");
		}
		if (!root) {
			root = std::make_shared<Node>(std::move(val));
			return;
		}
		std::shared_ptr<Node> parent, child{ root };
		while (parent = child, child = choose_child(parent,val.first));
		choose_child(parent, val.first) = std::make_shared<Node>(std::move(val), parent);

	}

	std::shared_ptr<Node> find_nearest(const K& key) {
		if (empty()) {
			throw logic_error("find_nearest in empty tree");
		}
		std::shared_ptr<Node> less{ root }, more{ root }, it{ root };
		while (it && it->key != key) {
			(key < it->key ? more : less) = it;
			it = choose_child(it, key);
		}
		if (it) { return it; }
		return key - less->key < more->key - key ? less : more; // TODO Abstract metrics
	}

	bool empty() {
		return !root;
	}

	template <class K, class V>
	friend std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst);

private:

	std::shared_ptr<Node>& choose_child(std::shared_ptr<Node> parent, const K& key) {
		return key < parent->key ? parent->left_child : parent->right_child;
	}

	void traverse(std::shared_ptr<Node> root, std::ostream& out) const {
		if (!root) { return; }
		traverse(root->left_child, out);
		out << root->value << " ";
		traverse(root->right_child, out);
	}

	void deep_copy(std::shared_ptr<Node>& target, std::shared_ptr<const Node> source, std::shared_ptr<Node> parent) {
		if (!source) { return; }
		target = std::make_shared<Node>(std::make_pair(source->key, source->value), std::move(parent));
		deep_copy(target->left_child, source->left_child, target);
		deep_copy(target->right_child, source->right_child, target);
	}

	K lw_bound, up_bound;
	std::shared_ptr<Node> root;
};

template <class K, class V>
std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst) {
	bst.traverse(bst.root, out);
	return out;
}