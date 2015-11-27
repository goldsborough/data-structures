#ifndef MAX_HEAP_HPP
#define MAX_HEAP_HPP

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>

template<typename T>
class MaxHeap
{
public:
	
	using size_t = std::size_t;
	
	static const size_t minimum_capacity;
	
	MaxHeap(size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(capacity)
	, _data(new T[capacity])
	{ }
	
	MaxHeap(const std::initializer_list<T>& list,
		 size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(std::max(capacity, list.size()))
	, _data(new T[capacity])
	{
		for (const auto& item : list) push(item);
	}
	
	MaxHeap(const MaxHeap& other)
	: _size(other._size)
	, _capacity(other._capacity)
	, _data(new T[_capacity])
	{
		std::copy(other._data, other._data + _size, _data);
	}
	
	MaxHeap(MaxHeap&& other)
	: MaxHeap()
	{
		swap(other);
	}
	
	MaxHeap& operator=(MaxHeap other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(MaxHeap& other)
	{
		using std::swap;
		
		swap(_size, other._size);
		
		swap(_capacity, other._capacity);
		
		swap(_data, other._data);
	}
	
	friend void swap(MaxHeap& first, MaxHeap& second)
	{
		first.swap(second);
	}
	
	~MaxHeap()
	{
		delete [] _data;
	}
	
	
	void push(const T& item)
	{
		if (++_size == _capacity) _resize();
		
		_data[_size] = item;
		
		_swim(_size);
	}
	
	const T& top() const
	{
		if (is_emtpy())
		{
			throw std::out_of_range("Nothing at top!");
		}
		
		return _data[1];
	}
	
	T pop()
	{
		if (is_emtpy())
		{
			throw std::out_of_range("Nothing to pop!");
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
	
	bool is_emtpy() const
	{
		return _size == 0;
	}
	
private:
	
	constexpr inline size_t _parent(size_t index) const
	{
		return index == 1 ? index : index/2;
	}
	
	constexpr inline size_t _left(size_t index) const
	{
		return 2 * index;
	}
	
	constexpr inline size_t _right(size_t index) const
	{
		return 2 * index + 1;
	}
	
	void _sink(size_t index)
	{
		while (index < _size)
		{
			auto left = _left(index);
			
			if (left > _size) return;
			
			auto right = _right(index);
			
			size_t child;
			
			if (right > _size || _data[left] >= _data[right]) child = left;
			
			else child = right;
			
			if (_data[child] > _data[index])
			{
				_swap(child, index);
			}
			
			else break;
			
			index = child;
		}
	}
	
	void _swim(size_t index)
	{
		auto parent = _parent(index);
		
		while (_data[parent] < _data[index])
		{
			_swap(parent, index);
			
			index = parent;
			
			parent = _parent(index);
		}
	}
	
	void _swap(size_t first, size_t second)
	{
		auto temp = _data[first];
		
		_data[first] = _data[second];
		_data[second] = temp;
	}
	
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
const typename MaxHeap<T>::size_t MaxHeap<T>::minimum_capacity = 10;

#endif /* MAX_HEAP_HPP */