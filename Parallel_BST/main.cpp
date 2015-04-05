#include "bst.hpp"
#include <vector>
#include <set>

using namespace std;

int main() {
	set<int> s;
	s.find(10);
	vector<int> data{ 45, 50, 25, 10, 5, 20, 15, 40, 30, 35, 200 };
	BST<int, int> m_bst(0, 100);
	for (int d : data) {
		try {
			m_bst.insert(std::make_pair(d, d));
		}
		catch (out_of_range e) {
			cerr << "error inserting {" << d << "," << d << "}: " << e.what() << endl;
		}
	}

	cout << m_bst.find_nearest(42)->key << " "
		 << m_bst.find_nearest(43)->key << " "
		 << m_bst.find_nearest(27)->key << " "
		 << m_bst.find_nearest(28)->key << " "
		 << m_bst.find_nearest(15)->key << endl;

	cout << m_bst;
	cin.ignore();
	return 0;
}