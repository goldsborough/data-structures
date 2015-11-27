#ifndef TRIE_HPP
#define TRIE_HPP

#include <array>
#include <cstddef>
#include <string>

template<typename Value, typename String = std::string, std::size_t N = 128>
class Trie
{
public:
	
	using size_t = std::size_t;
	
	Trie()
	: _size(0)
	, _root(nullptr)
	{ }
	
	Trie(std::initializer_list<std::pair<String, Value>> list)
	: Trie()
	{
		for (const auto& item : list)
		{
			insert(item.first, item.second);
		}
	}
	
	Trie(const Trie& other)
	: _size(other._size)
	, _root(_copy(other._root))
	{ }
	
	Trie(Trie&& other) noexcept
	: Trie()
	{
		swap(other);
	}
	
	Trie& operator=(Trie other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(Trie& other) noexcept
	{
		// Enable ADL
		using std::swap;
		
		swap(_root, other._root);
		
		swap(_size, other._size);
	}
	
	friend void swap(Trie& first, Trie& second) noexcept
	{
		first.swap(second);
	}
	
	~Trie()
	{
		_clear(_root);
	}
	
	
	void insert(const String& key, const Value& value)
	{
		_root = _insert(_root, key, value);
	}
	
	
	Value& operator[](const String& key)
	{
		auto node = _find(_root, key);
		
		if (! node)
		{
			node = new Node;
			
			_root = _insert(_root, key, node);
		}
		
		return node->value;
	}
	
	
	Value& get(const String& key)
	{
		auto node = _find(_root, key);
		
		if (! node)
		{
			throw std::invalid_argument("No such key!");
		}
		
		return node->value;
	}
	
	const Value& get(const String& key) const
	{
		auto node = _find(_root, key);
		
		if (! node)
		{
			throw std::invalid_argument("No such key!");
		}
		
		return node->value;
	}
	
	
	bool contains(const String& key)
	{
		return _find(_root, key) != nullptr;
	}
	
	
	void erase(const String& key)
	{
		_root = _erase(_root, key);
	}
	
	void clear()
	{
		_clear(_root);
		
		_root = nullptr;
		
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
		Node(const Value& value_ = Value(),
			 bool has_value_ = false)
		: value(value_)
		, has_value(has_value_)
		{
			std::fill(next.begin(), next.end(), nullptr);
		}
		
		Value value;
		
		bool has_value;
		
		std::array<Node*, N> next;
	};
	
	Node* _insert(Node* node, const String& key, const Value& value, size_t index = 0)
	{
		if (! node) node = new Node(value);
		
		if (index == key.length())
		{
			if (! node->has_value)
			{
				++_size;
				
				node->has_value = true;
			}
			
			node->value = value;
		}
		
		else
		{
			auto& next = node->next[key[index]];
			
			next = _insert(next, key, value, ++index);
		}
		
		return node;
	}
	
	Node* _insert(Node* node, const String& key, Node* new_node, size_t index = 0)
	{
		if (index == key.length())
		{
			++_size;
			
			return new_node;
		}
		
		else if (! node) node = new Node;
		
		auto& next = node->next[key[index]];
		
		next = _insert(next, key, new_node, ++index);
		
		return node;
	}
	
	Node* _find(Node* node, const String& key, size_t index = 0)
	{
		if (! node) return nullptr;
		
		if (index == key.length())
		{
			return node->has_value ? node : nullptr;
		}
		
		auto& next = node->next[key[index]];
		
		return _find(next, key, ++index);
	}
	
	Node* _erase(Node* node, const String& key, size_t index = 0)
	{
		if (! node) throw std::invalid_argument("No such key!");
		
		if (index == key.length())
		{
			if (! node->has_value)
			{
				throw std::invalid_argument("No such key!");
			}
			
			if (std::any_of(node->next.begin(), node->next.end(),
							[] (Node* node) { return node != nullptr; }))
			{
				node->has_value = false;
			}
			
			else
			{
				delete node;
				
				node = nullptr;
			}
			
			--_size;
		}
		
		else
		{
			auto& next = node->next[key[index]];
			
			next = _erase(next, key, ++index);
			
		}
		
		return node;
	}
	
	void _clear(Node* node)
	{
		if (! node) return;
		
		for (auto& next : node->next) _clear(next);
		
		delete node;
	}
	
	Node* _copy(Node* other)
	{
		if (! other) return nullptr;
		
		auto node = new Node(other->value, other->has_value);
		
		for (size_t i = 0; i < N; ++i)
		{
			node->next[i] = _copy(other->next[i]);
		}
		
		return node;
	}
	
	Node* _root;
	
	size_t _size;
};


#endif /* TRIE_HPP */