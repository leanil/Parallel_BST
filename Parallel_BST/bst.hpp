#include <memory>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <functional>

template <class K, class V>
class BST
{
private:

	struct Node
	{
		Node(std::pair<K, V> val, typename BST::Node_ptr parent)
			: val{ std::move(val) }, red{ true }, parent{ std::move(parent) } {}
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

	// Ineffective if K is not MoveConstructible.
	BST(K lb, K ub) : lw_bound{ std::move(lb) }, up_bound{ std::move(ub) } {}

	BST(const BST& other) :lw_bound{ other.lw_bound }, up_bound{ other.up_bound } {
		deep_copy(root, other.root, Node_ptr());
	}

	BST(BST&& other) : lw_bound{ std::move(other.lw_bound) }, up_bound{ std::move(other.up_bound) }, root{ std::move(other.root) } {}

	BST& operator=(BST other) {
		swap(other);
		return *this;
	}

	bool empty() {
		return !root;
	}

	void insert(std::pair<K, V> val) {
		if (val.first < lw_bound || val.first >= up_bound) {
			throw std::out_of_range("key is out of boundaries");
		}
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
			if (parent(act) == grand_parent(act)->left_child) {
				restore(act, left_child, right_child);
			}
			else {
				restore(act, right_child, left_child);
			}
		}
		root->red = false;
	}

	typename BST::Node_ptr find_nearest(const K& key) {
		if (empty()) {
			throw logic_error("find_nearest in empty tree");
		}
		Node_ptr less{ root }, more{ root }, it{ root };
		while (it && it->key != key) {
			(key < it->key ? more : less) = it;
			it = choose_child(it, key);
		}
		if (it) { return it; }
		return key - less->key < more->key - key ? less : more; // TODO Abstract metrics
	}

	void swap(BST& other) {
		std::swap(lw_bound, other.lw_bound);
		std::swap(up_bound, other.up_bound);
		std::swap(root, other.root);
	}

	template <class K, class V>
	friend std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst);

private:

	Node_ptr& choose_child(const Node_ptr& parent, const K& key) {
		return key < parent->key() ? parent->left_child : parent->right_child;
	}

	Node_ptr parent(const Node_ptr& child) {
		return child ? child->parent.lock() : child;
	}

	Node_ptr grand_parent(const Node_ptr& child) {
		return parent(child) ? parent(child)->parent.lock() : parent(child);
	}

	void traverse(Node_ptr root, std::ostream& out) const {
		if (!root) { return; }
		traverse(root->left_child, out);
		print<K,V>(out,*root);
		traverse(root->right_child, out);
	}

	void deep_copy(Node_ptr& target, std::shared_ptr<const Node> source, Node_ptr parent) {
		if (!source) { return; }
		target = std::make_shared<Node>(source->val, std::move(parent));
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
};

template <class K, class V>
std::ostream& operator<<(std::ostream& out, const BST<K, V>& bst) {
	bst.traverse(bst.root, out);
	return out;
}

template <class K, class V>
std::ostream& print(std::ostream& out,typename BST<K,V>::Node node) {
	out << node.key() << " " << (node.left_child ? node.left_child->key() : -1) << " "
		<< (node.right_child ? node.right_child->key() : -1) << " "
		<< (node.parent.lock() ? node.parent.lock()->key() : -1) << " "
		<< (node.red?"p":"f") << endl;
	return out;
}