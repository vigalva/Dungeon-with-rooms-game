#include "CompareNewNodes.h"



CompareNewNodes::CompareNewNodes()
{
}


CompareNewNodes::~CompareNewNodes()
{
}


bool CompareNewNodes::operator ( )(const NewNode& n1, const NewNode& n2)
{
	return n1.GetF() > n2.GetF();
}
