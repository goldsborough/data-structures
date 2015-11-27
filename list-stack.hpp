#ifndef LIST_STACK_HPP
#define LIST_STACK_HPP

#include <initializer_list>
#include <stdexcept>

template<typename T>
class ListStack
{
public:
	
	using size_t = std::size_t;
	
	ListStack()
	: _size(0)
	, _first(nullptr)
	{ }
	
	ListStack(std::initializer_list<T> list)
	: ListStack()
	{
		for (const auto& item : list) push(item);
	}
	
	ListStack(const ListStack& other)
	: _size(0)
	, _first(nullptr)
	{
		for (auto node = other._first; node; node = node->next)
		{
			push(node->item);
		}
	}
	
	ListStack(ListStack&& other) noexcept
	: ListStack()
	{
		swap(other);
	}
	
	ListStack& operator=(ListStack other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(ListStack& other) noexcept
	{
		// Enable ADL
		using std::swap;
		
		swap(_first, other._first);
		
		swap(_size, other._size);
	}
	
	friend void swap(ListStack& first, ListStack& second) noexcept
	{
		first.swap(second);
	}
	
	~ListStack()
	{
		clear();
	}
	
	
	void push(const T& item)
	{
		auto node = new Node(item, _first);
		
		_first = node;
		
		++_size;
	}
	
	
	T& top()
	{
		if (! _first)
		{
			throw std::out_of_range("No element at top of stack!");
		}
		
		return _first->item;
	}
	
	const T& top() const
	{
		if (! _first)
		{
			throw std::out_of_range("No element at top of stack!");
		}
		
		return _first->item;
	}
	
	
	T pop()
	{
		if (! _first)
		{
			throw std::out_of_range("No element at top of stack!");
		}
		
		auto node = _first;
		
		_first = _first->next;
		
		auto item = node->item;
		
		delete node;
		
		--_size;
		
		return item;
	}
	
	
	void clear()
	{
		for (Node* next; _first; _first = next)
		{
			next = _first->next;
			
			delete _first;
		}
		
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
	
	
	Node* _first;
	
	size_t _size;
};

#endif /* LIST_STACK_HPP */