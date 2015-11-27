#ifndef HEAP_FILTER_HPP
#define HEAP_FILTER_HPP

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <utility>

template<typename T, std::size_t N>
class HeapFilter
{
public:
	
	using size_t = std::size_t;
	
	using compare_t = std::function<bool(const T&, const T&)>;
	
	
	HeapFilter(compare_t compare = std::greater<T>())
	: _size(0)
	, _compare(compare)
	{
		std::fill(_data, _data + N + 1, 0);
	}
	
	HeapFilter(std::initializer_list<T> list,
			   compare_t compare = std::greater<T>())
	: HeapFilter()
	{
		for (const auto& item : list) push(item);
	}
	
	HeapFilter(const HeapFilter& other)
	: _size(other._size)
	{
		std::copy(other._data, other._data + _size, _data);
	}
	
	HeapFilter(HeapFilter&& other) noexcept
	: HeapFilter()
	{
		swap(other);
	}
	
	HeapFilter& operator=(HeapFilter other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(HeapFilter& other)
	{
		// Enable ADL
		using std::swap;
		
		swap(_data, other._data);
		
		swap(_size, other._size);
	}
	
	friend void swap(HeapFilter& first, HeapFilter& second)
	{
		first.swap(second);
	}
	
	~HeapFilter() = default;
	
	
	void push(const T& item)
	{
		if (_size == N && _compare(item, _data[1]))
		{
			_data[1] = item;
			
			_sink(1);
		}
		
		else
		{
			_data[++_size] = item;
			
			_swim(_size);
		}
	}
	
	
	T& top()
	{
		return _data[1];
	}
	
	const T& top() const
	{
		if (is_empty())
		{
			throw std::invalid_argument("No element in heap!");
		}
		
		return _data[1];
	}
	
	
	T pop()
	{
		if (is_empty())
		{
			throw std::invalid_argument("No element in heap!");
		}
		
		auto item = _data[1];
		
		_swap(1, _size--);
		
		_sink(1);
		
		return item;
	}
	
	size_t size() const
	{
		return _size;
	}
	
	bool is_empty() const
	{
		return _size == 0;
	}
	
private:
	
	constexpr inline size_t _left_child(size_t index) const
	{
		return 2 * index;
	}
	
	constexpr inline size_t _right_child(size_t index) const
	{
		return 2 * index + 1;
	}
	
	constexpr inline size_t _parent(size_t index) const
	{
		return index == 1 ? 1 : index/2;
	}
	
	void _sink(size_t index)
	{
		while (index < _size)
		{
			auto left = _left_child(index);
			
			if (left > _size) return;
			
			auto right = _right_child(index);
			
			size_t child;
			
			if (right > _size || _compare(_data[right], _data[left])) child = left;
			
			else child = right;
			
			if (! _compare(_data[index], _data[child])) return;
			
			_swap(child, index);
			
			index = child;
		}
	}

	void _swim(size_t index)
	{
		auto parent = _parent(index);
		
		while (_compare(_data[parent], _data[index]))
		{
			_swap(index, parent);
			
			index = parent;
			
			parent = _parent(index);
		}
	}
	
	void _swap(size_t first, size_t second)
	{
		std::swap(_data[first], _data[second]);
	}
	
	
	T _data [N+1];
	
	size_t _size;
	
	compare_t _compare;
};


#endif /* HEAP_FILTER_HPP */