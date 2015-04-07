#include "bst.hpp"
#include <random>
#include <chrono>
#include <algorithm>

using namespace std;

int main() {

#pragma region basic functions
	/*vector<int> data{ 45, 50, 25, 10, 5, 20, 15, 40, 30, 35, 200 };
	BST<int, int> m_bst(0, 100);
	for (int d : data) {
		try {
			m_bst.insert(make_pair(d, d));
		}
		catch (out_of_range e) {
			cerr << "error inserting {" << d << "," << d << "}: " << e.what() << endl;
		}
	}

	cout << m_bst.find_nearest(42)->key << " "
		 << m_bst.find_nearest(43)->key << " "
		 << m_bst.find_nearest(27)->key << " "
		 << m_bst.find_nearest(28)->key << " "
		 << m_bst.find_nearest(15)->key << endl;*/
#pragma endregion

#pragma region implicit destructor (does not leak)
	/*uniform_int_distribution<> dist(0, 100);
	default_random_engine rand;
	for(;;){
		BST<long long, long long> huge_bst(0,100);
		for (int i = 0; i < 1000; i++) {
			huge_bst.insert(make_pair(dist(rand), i));
		}
	}*/
#pragma endregion

#pragma region Copy/Move constructor/assignment
	/*BST<int, int> m_bst2(0,1);
	m_bst2 = m_bst;
	cout << m_bst << endl;
	m_bst2.insert(make_pair(0, 0));
	cout << m_bst << endl;
	cout << m_bst2 << endl;*/
#pragma endregion

#pragma region insert benchmark
	/*BST<int, int> deformed(0, 10000);
	auto start = chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; i++) {
		deformed.insert(make_pair(i, i));
	}
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
	cout << duration << endl;*/
#pragma endregion

// TODO run when iterators are done
#pragma region sorting check 
	/*uniform_int_distribution<> dist(0, 1000000);
	default_random_engine rand;
	BST<int, int> m_bst(0, 1000000);
	vector<int> values;
	for (int i = 0; i < 10000; i++) {
		int x = dist(rand);
		m_bst.insert(make_pair(x, 0));
		values.push_back(x);
	}
	sort(values.begin(), values.end());
	vector<int> bst_values(m_bst.begin(), m_bst.end());
	cout << (bst_values == values);*/
#pragma endregion

#pragma region parallel find
	BST<int, int> m_bst(0, 1000000);
	vector<int> query(1000000);
	for (int i = 0; i < 1000000; i++) {
		query[i] = i;
		m_bst.insert(make_pair(i, i));
	}
	auto start = chrono::high_resolution_clock::now();
	auto result = m_bst.find_nearest(query);
	auto end = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
	cout << duration << " " << result[0]->key() << endl;

#pragma endregion
	return 0;
}