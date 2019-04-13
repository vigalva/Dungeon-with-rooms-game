#pragma once
#include "NewNode.h"
class CompareNewNodes
{
public:
	CompareNewNodes();
	~CompareNewNodes();
	bool operator ( ) (const NewNode& n1, const NewNode& n2);
};