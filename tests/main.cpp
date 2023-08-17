#include <covscript/covscript.hpp>

int main(int argc, const char **args)
{
	if (argc != 2)
		return -1;
	cs::extension dll(args[1]);
	cs::callable func = dll.get_var("print").const_val<cs::callable>();
	cs::invoke(func, "Hello");
	return 0;
}
