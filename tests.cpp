#include <iostream>
#include <string>
#include <utility>
#include <memory>

std::string& function()
{
	static std::string lol;
	return lol;
}

std::string otherFunction()
{
	return std::string{};
}

void lvalueFunction()
{

	std::string obj;
	obj;            // lvalue

	std::string& function();
	function();     // lvalue

	std::string&& rvalRef = std::move(obj);
	rvalRef;        // lvalue

}

void xvalueFunction()
{

	std::string obj;
	std::move(obj);                 // xvalue

	std::pair<int, int>{}.first;    // xvalue

	std::pair<int, int> pair;
	std::move(pair).first;          // xvalue

}

struct jsp
{
	void prvalueFunction()
	{

		std::string{};      // prvalue

		std::string otherFunction();
		otherFunction();    // prvalue

	}
};

class VerboseParameter
{
public:
	VerboseParameter() { std::cout << "VerboseParameter default constructor." << std::endl; }
	VerboseParameter(const VerboseParameter& other) { std::cout << "VerboseParameter copy constructor." << std::endl; }
	VerboseParameter(VerboseParameter&& other) noexcept { std::cout << "VerboseParameter move constructor." << std::endl; }
};

class VerboseClass
{
public:
	VerboseClass() { std::cout << "VerboseClass default constructor." << std::endl; }
	VerboseClass(int newValue) : value{ newValue } { std::cout << "VerboseClass arg constructor. (" << value << ": " << this << ")" << std::endl; }
	VerboseClass(const VerboseClass& other) { std::cout << "VerboseClass copy constructor." << std::endl; }
	VerboseClass(VerboseClass&& other) noexcept {
		if (other.value != 0)
			std::cout << "VerboseClass move constructor. (" << other.value << ": " << this << ")" << std::endl;
		else
			std::cout << "VerboseClass move constructor." << std::endl;
	}
	VerboseClass(VerboseParameter&& other) { std::cout << "VerboseClass parameter move constructor." << std::endl; }
	VerboseClass(const VerboseParameter& other) { std::cout << "VerboseClass parameter copy constructor." << std::endl; }

	int value = 0;
};

VerboseClass buildFromClass()
{
	VerboseClass other;
	return other;
}

VerboseClass buildFromParamter()
{
	VerboseParameter param;
	return param;
}

VerboseClass iBuildStuff()
{
	return VerboseClass{};
}

void basicExemple()
{
	VerboseClass imStuff = iBuildStuff();
}

VerboseClass iBuildMoreStuff()
{
	VerboseClass notStuff{ 21 };
	return VerboseClass{ 25 };
}

void advancedExemple()
{
	VerboseClass alsoNotStuff{ 11 };
	VerboseClass imStuff = iBuildMoreStuff();
	VerboseClass stillNotStuff{ 12 };
}

int main()
{
	//basicExemple();
	VerboseClass imStuff = iBuildStuff();
}