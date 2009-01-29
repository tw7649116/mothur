#ifndef NAMEASSIGNMENT_HPP
#define NAMEASSIGNMENT_HPP

using namespace std;

#include <string>
#include <map>
#include "utilities.hpp"
#include "listvector.hpp"

class NameAssignment : public map<string,int> {
public:
	NameAssignment(string);
	void readMap(int, int);
	ListVector getListVector();
	int get(string);
	void print();
private:
	ifstream fileHandle;
	ListVector list;
};




#endif