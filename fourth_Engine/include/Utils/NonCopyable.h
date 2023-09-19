#pragma once

namespace fth
{
	class NonCopyable
	{

		// -------------- Note ---------
		//Deleted methods should be public: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-delete
	public:
		NonCopyable() = default;
		NonCopyable(const NonCopyable& other) = delete;
		NonCopyable& operator=(const NonCopyable& other) = delete;
	};
}