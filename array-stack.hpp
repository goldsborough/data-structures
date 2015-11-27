#ifndef ARRAY_STACK_HPP
#define ARRAY_STACK_HPP

#include <initializer_list>
#include <stdexcept>

template<typename T>
class ArrayStack
{
public:
	
	using size_t = std::size_t;
	
	static const size_t minimum_capacity;
	
	ArrayStack(size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(capacity)
	, _data(new T[_capacity])
	{ }
	
	ArrayStack(std::initializer_list<T> list,
			   size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(std::max(list.size(), capacity))
	, _data(new T[_capacity])
	{
		for (const auto& item : list) push(item);
	}
	
	ArrayStack(const ArrayStack& other)
	: _size(other._size)
	, _capacity(other._capacity)
	, _data(new T[other._capacity])
	{
		std::copy(other._data, other._data + _size, _data);
	}
	
	ArrayStack(ArrayStack&& other) noexcept
	: ArrayStack()
	{
		swap(other);
	}
	
	ArrayStack& operator=(ArrayStack other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(ArrayStack& other)
	{
		// Enable ADL
		using std::swap;
		
		swap(_size, other._size);
		
		swap(_capacity, other._capacity);
		
		swap(_data, other._data);
	}
	
	friend void swap(ArrayStack& first, ArrayStack& second)
	{
		first.swap(second);
	}
	
	~ArrayStack()
	{
		delete [] _data;
	}
	
	
	void push(const T& item)
	{
		_data[_size] = item;
		
		if (++_size == _capacity) _resize();
	}
	
	
	T& top()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at top of stack!");
		}
		
		return _data[_size - 1];
	}
	
	const T& top() const
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at top of stack!");
		}
		
		return _data[_size - 1];
	}
	
	
	T pop()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at top of stack!");
		}
		
		auto item = _data[_size - 1];
		
		if (--_size == _capacity/4) _resize();
		
		return item;
	}
	
	void clear()
	{
		delete [] _data;
		
		_size = 0;
		
		_capacity = minimum_capacity;
		
		_data = new T[_capacity];
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
	
	void _resize()
	{
		_capacity = std::max(_size * 2, minimum_capacity);
		
		auto old = _data;
		
		_data = new T[_capacity];
		
		std::copy(old, old + _size, _data);
		
		delete [] old;
	}
	
	size_t _size;
	
	size_t _capacity;
	
	T* _data;
};

template<typename T>
const typename ArrayStack<T>::size_t ArrayStack<T>::minimum_capacity = 8;

#endif /* ARRAY_STACK_HPP */