#ifndef ARRAY_QUEUE_HPP
#define ARRAY_QUEUE_HPP

#include <initializer_list>
#include <stdexcept>

template<typename T>
class ArrayQueue
{
public:
	
	using size_t = std::size_t;
	
	static const size_t minimum_capacity;
	
	
	ArrayQueue(size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(capacity)
	, _front(0)
	, _back(0)
	, _data(new T[capacity])
	{ }
	
	ArrayQueue(std::initializer_list<T> list,
			   size_t capacity = minimum_capacity)
	: _size(0)
	, _capacity(std::max(list.size(), capacity))
	, _front(0)
	, _back(0)
	, _data(new T[_capacity])
	{
		for (const auto& item : list) enqueue(item);
	}
	
	ArrayQueue(const ArrayQueue& other)
	: _size(0)
	, _capacity(other._size * 2)
	, _front(0)
	, _back(0)
	,_data(new T[_capacity])
	{
		for (auto i = other._front; i != other._back; i = (i + 1) % other._capacity)
		{
			enqueu(other._data[i]);
		}
	}
	
	ArrayQueue(ArrayQueue&& other) noexcept
	: ArrayQueue()
	{
		swap(other);
	}
	
	ArrayQueue& operator=(ArrayQueue other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(ArrayQueue& other) noexcept
	{
		// Enable ADL
		using std::swap;
		
		swap(_size, other._size);
		
		swap(_capacity, other._capacity);
		
		swap(_front, other._front);
		
		swap(_back, other._back);
		
		swap(_data, other._data);
	}
	
	friend void swap(ArrayQueue& first, ArrayQueue& second) noexcept
	{
		first.swap(second);
	}
	
	~ArrayQueue()
	{
		delete [] _data;
	}
	
	
	void enqueue(const T& item)
	{
		_back = (_back + 1) % _capacity;
		
		if (++_size == _capacity) _resize();
		
		_data[_back] = item;
	}
	
	
	T& front()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at front of queue!");
		}
		
		return _data[_front];
	}
	
	const T& front() const
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at front of queue!");
		}
		
		return _data[_front];
	}
	
	
	T& back()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at back of queue!");
		}
		
		return _data[_back];
	}
	
	const T& back() const
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at front of queue!");
		}
		
		return _data[_back];
	}
	
	
	T dequeue()
	{
		auto item = _data[_front];
		
		_front = (_front + 1) % _capacity;
		
		if (--_size == _capacity/4) _resize();
		
		return item;
	}
	
	void clear()
	{
		delete [] _data;
		
		_capacity = minimum_capacity;
		
		_data = new T[_capacity];
		
		_size = _back = _front = 0;
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
		_capacity = std::max(minimum_capacity, _size * 2);
		
		auto old = _data;
		
		_data = new T[_capacity];
		
		for (size_t i = 0; _front != _back; ++i, _front = (_front + 1) % _capacity)
		{
			_data[i] = old[_front];
		}
		
		_front = 0;
		
		_back = _size - 1;
		
		delete [] old;
	}
	
	size_t _size;
	
	size_t _capacity;
	
	size_t _front;
	
	size_t _back;
	
	T* _data;
};

template<typename T>
const typename ArrayQueue<T>::size_t ArrayQueue<T>::minimum_capacity = 8;

#endif /* ARRAY_QUEUE_HPP */