module;

#include <array>
#include <memory>
#include <vector>

export module ArxConverter.Container;


export import ArxConverter.Types;


export template<typename Type, Count count>
using Array = std::array<Type, count>;

export template<typename Type>
using DynamicArray = std::vector<Type>;


export template<typename Type>
using Unique = std::unique_ptr<Type>;