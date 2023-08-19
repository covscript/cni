#include <covscript/covscript.hpp>

int main(int argc, const char **args)
{
	if (argc != 2)
		return -1;
	cs::extension dll(args[1]);
	cs::function_invoker<void(std::string)> func1(dll.get_var("print"));
	func1("Hello");
	cs::var func2 = dll.get_var("print");
	cs::invoke(func2, "Hello");
	return 0;
}
