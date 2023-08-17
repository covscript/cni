#include <covscript/dll.hpp>
#include <covscript/cni.hpp>
#include <iostream>

CNI_ROOT_NAMESPACE {
	void print(const std::string& str)
	{
		std::cout << str << std::endl;
	}
	CNI(print)
}
