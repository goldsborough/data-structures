#ifndef MIN_HEAP_HPP
#define MIN_HEAP_HPP

#include <algorithm>
#include <cstddef>

template<typename T>
class MinHeap
{
public:
	
	using size_t = std::size_t;
	
	static const size_t minimum_capacity;
	
	
	MinHeap(size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(capacity)
	, _data(new T[_capacity])
	{ }
	
	MinHeap(std::initializer_list<T> list,
			size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(std::max(capacity, list.size()))
	, _data(new T[_capacity])
	{
		for (const auto& item : list) push(item);
	}
	
	MinHeap(const MinHeap& other)
	: _size(other._size)
	, _capacity(other._capacity)
	, _data(new T[_capacity])
	{
		for (size_t i = 1; i <= _size; ++i)
		{
			_data[i] = other._data[i];
		}
	}
	
	MinHeap(MinHeap&& other) noexcept
	: MinHeap()
	{
		swap(other);
	}
	
	MinHeap& operator=(MinHeap other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(MinHeap& other)
	{
		// Enable ADL
		using std::swap;
		
		swap(_size, other._size);
		
		swap(_capacity, other._capacity);
		
		swap(_data, other._data);
	}
	
	friend void swap(MinHeap& first, MinHeap& second)
	{
		first.swap(second);
	}
	
	~MinHeap()
	{
		delete [] _data;
	}
	
	
	void push(const T& item)
	{
		if (++_size == _capacity) _resize();
		
		_data[_size] = item;
		
		_swim(_size);
	}
	
	
	T& top()
	{
		if (is_empty())
		{
			throw std::out_of_range("Nothing at top of heap!");
		}
		
		return _data[1];
	}
	
	const T& top() const
	{
		if (is_empty())
		{
			throw std::out_of_range("Nothing at top of heap!");
		}
		
		return _data[1];
	}
	
	
	T pop()
	{
		if (is_empty())
		{
			throw std::out_of_range("Nothing at top of heap!");
		}
		
		auto item = _data[1];
		
		_swap(1, _size);
		
		if (--_size == _capacity/4) _resize();
		
		_sink(1);
		
		return item;
	}
	
	void clear()
	{
		delete [] _data;
		
		_capacity = minimum_capacity;
		
		_data = new T[_capacity];
		
		_size = 0;
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
	
	void _swim(size_t index)
	{
		auto parent = _parent(index);
		
		while (_data[parent] > _data[index])
		{
			_swap(parent, index);
			
			index = parent;
			
			parent = _parent(index);
		}
	}
	
	void _sink(size_t index)
	{
		while (index < _size)
		{
			auto left = _left_child(index);
			
			if (left > _size) return;
			
			auto right = _right_child(index);
			
			size_t child;
			
			if (right > _size || _data[left] < _data[right]) child = left;
			
			else child = right;
			
			if (_data[child] < _data[index])
			{
				_swap(child, index);
			}
			
			else break;
			
			index = child;
		}
	}
	
	void _swap(size_t first, size_t second)
	{
		std::swap(_data[first], _data[second]);
	}
	
	void _resize()
	{
		_capacity = std::max(minimum_capacity, _size * 2);
		
		auto old = _data;
		
		_data = new T[_capacity];
		
		std::copy(old, old + _size + 1, _data);
		
		delete [] old;
	}
	
	
	size_t _size;
	
	size_t _capacity;
	
	T* _data;
	
};

template<typename T>
const typename MinHeap<T>::size_t MinHeap<T>::minimum_capacity = 8;

#endif /* MIN_HEAP_HPP */