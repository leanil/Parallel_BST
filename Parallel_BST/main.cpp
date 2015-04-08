#include "bst.hpp"
#include <random>
#include <chrono>
#include <algorithm>

using namespace std;

int main() {
	BST<int, int> m_bst(0, 0);
#pragma region basic functions
	vector<int> data{ 45, 50, 25, 10, 5, 20, 15, 40, 30, 35, 200 };
	m_bst = BST<int, int>(0, 100);
	for (int d : data) {
		m_bst.insert(make_pair(d, d));
	}

	cout << m_bst.find_nearest(42)->first << " "
		<< m_bst.find_nearest(43)->first << " "
		<< m_bst.find_nearest(27)->first << " "
		<< m_bst.find_nearest(28)->first << " "
		<< m_bst.find_nearest(15)->first << endl;
	cout << m_bst << endl;
	m_bst.change_bounds(15, 40);
	for (auto it : m_bst) {
		cout << "{" << it.first << "," << it.second << "} ";
	}
	cout << endl << endl;
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
	BST<int, int> m_bst2(0, 1);
	m_bst2 = m_bst;
	m_bst2.insert(make_pair(39, 39));
	cout << m_bst << endl;
	cout << m_bst2 << endl;
	m_bst = move(m_bst2);
	cout << m_bst2 << endl;
	cout << m_bst << endl << endl;
#pragma endregion

#pragma region insert benchmark
	BST<int, int> deformed(0, 10000);
	auto start1 = chrono::high_resolution_clock::now();
	for (int i = 0; i < 10000; i++) {
		deformed.insert(make_pair(i, i));
	}
	auto end1 = chrono::high_resolution_clock::now();
	auto duration1 = chrono::duration_cast<chrono::milliseconds>(end1 - start1).count();
	cout << duration1 << endl << endl;
#pragma endregion

#pragma region sorting check 
	uniform_int_distribution<> dist(0, 1000000);
	default_random_engine rand;
	m_bst = BST<int, int>(0, 1000000);
	vector<int> values;
	for (int i = 0; i < 10000; i++) {
		int x = dist(rand);
		m_bst.insert(make_pair(x, 0));
		values.push_back(x);
	}
	sort(values.begin(), values.end());
	vector<int> bst_values(values.size());
	transform(m_bst.begin(), m_bst.end(), bst_values.begin(), [] (pair<int, int> p) {return p.first; });
	cout << (bst_values == values) << endl << endl;
#pragma endregion

#pragma region parallel find
	m_bst = BST<int, int>(0, 100000);
	vector<int> query(100000);
	for (int i = 0; i < 100000; i++) {
		query[i] = i;
		m_bst.insert(make_pair(i, i));
	}
	auto start2 = chrono::high_resolution_clock::now();
	auto result = m_bst.find_nearest(query);
	auto end2 = chrono::high_resolution_clock::now();
	auto duration2 = chrono::duration_cast<chrono::milliseconds>(end2 - start2).count();
	cout << duration2 << " " << result[0]->first << endl;

#pragma endregion
	return 0;
}