#ifndef LIST_QUEUE_HPP
#define LIST_QUEUE_HPP

#include <initializer_list>
#include <stdexcept>

template<typename T>
class ListQueue
{
public:
	
	using size_t = std::size_t;
	
	ListQueue()
	: _front(nullptr)
	, _back(nullptr)
	, _size(0)
	{ }
	
	ListQueue(std::initializer_list<T> list)
	: ListQueue()
	{
		for (const auto& item : list) enqueue(item);
	}
	
	ListQueue(const ListQueue& other)
	: ListQueue()
	{
		for (auto node = other._first; node; node = node->next)
		{
			enqueue(node->item);
		}
	}
	
	ListQueue(ListQueue&& other) noexcept
	: ListQueue()
	{
		swap(other);
	}
	
	ListQueue& operator=(ListQueue other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(ListQueue& other) noexcept
	{
		using std::swap;
		
		swap(_front, other._front);
		
		swap(_back, other._back);
		
		swap(_size, other._size);
	}
	
	friend void swap(ListQueue& first, ListQueue& second) noexcept
	{
		first.swap(second);
	}
	
	~ListQueue()
	{
		clear();
	}
	
	
	void enqueue(const T& item)
	{
		auto node = new Node(item);
		
		if (_back) _back->next = node;
		
		else _front = node;
		
		_back = node;
		
		++_size;
	}
	
	
	T& front()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at front of queue!");
		}
		
		return _front->item;
	}
	
	const T& front() const
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at front of queue!");
		}
		
		return _front->item;
	}
	
	
	T& back()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at back of queue!");
		}
		
		return _back->item;
	}
	
	const T& back() const
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at back of queue!");
		}
		
		return _back->item;
	}
	
	
	T dequeue()
	{
		if (is_empty())
		{
			throw std::out_of_range("No element at front of queue!");
		}
		
		auto node = _front;
		
		_front = _front->next;
		
		auto item = node->item;
		
		delete node;
		
		--_size;
		
		return item;
	}
	
	void clear()
	{
		for (Node* next; _front; _front = next)
		{
			next = _front->next;
			
			delete _front;
		}
		
		_back = nullptr;
		
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
	
	struct Node
	{
		Node(const T& item_,
			 Node* next_ = nullptr)
		: item(item_)
		, next(next_)
		{ }
		
		T item;
		
		Node* next;
	};
	
	
	Node* _front;
	
	Node* _back;
	
	size_t _size;
};

#endif /* LIST_QUEUE_HPP */