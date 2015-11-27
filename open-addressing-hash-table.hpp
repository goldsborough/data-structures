#ifndef OPEN_ADDRESSING_HASH_TABLE_HPP
#define OPEN_ADDRESSING_HASH_TABLE_HPP

#include <algorithm>
#include <functional>
#include <stdexcept>

template<typename Key, typename Value>
class OpenAddressingHashTable
{
public:
	
	using size_t = std::size_t;
	
	using pre_hash_t = std::function<size_t(const Key&)>;
	
	static const size_t minimum_capacity = 20;
	
	
	OpenAddressingHashTable(size_t capacity = minimum_capacity,
							const pre_hash_t& pre_hash = std::hash<Key>())
	: _size(0)
	, _capacity(capacity)
	, _pre_hash(pre_hash)
	, _nodes(new Node*[_capacity])
	{
		std::fill(_nodes, _nodes + _capacity, nullptr);
	}
	
	OpenAddressingHashTable(std::initializer_list<std::pair<Key, Value>> list,
							size_t capacity = minimum_capacity,
							const pre_hash_t& pre_hash = std::hash<Key>())
	: _size(0)
	, _capacity(std::max(capacity, list.size()))
	, _pre_hash(pre_hash)
	, _nodes(new Node*[_capacity])
	{
		std::fill(_nodes, _nodes + _capacity, nullptr);
		
		for (const auto& item : list)
		{
			insert(item.first, item.second);
		}
	}
	
	OpenAddressingHashTable(const OpenAddressingHashTable& other)
	: _size(0)
	, _capacity(other._capacity)
	, _pre_hash(other._pre_hash)
	, _nodes(new Node*[other._capacity])
	{
		std::fill(_nodes, _nodes + _capacity, nullptr);
		
		for (size_t i = 0; i < other._capacity; ++i)
		{
			if (other._nodes[i] && other._nodes[i]->is_alive)
			{
				insert(other._nodes[i]->key,
					   other._nodes[i]->key);
			}
		}
	}
	
	OpenAddressingHashTable(OpenAddressingHashTable&& other) noexcept
	: OpenAddressingHashTable()
	{
		swap(other);
	}
	
	OpenAddressingHashTable& operator=(OpenAddressingHashTable other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(OpenAddressingHashTable& other) noexcept
	{
		using std::swap;
		
		swap(_nodes, other._nodes);
		
		swap(_size, other._size);
		
		swap(_capacity, other._capacity);
		
		swap(_pre_hash, other._pre_hash);
	}
	
	friend void swap(OpenAddressingHashTable& first,
					 OpenAddressingHashTable& second) noexcept
	{
		first.swap(second);
	}
	
	~OpenAddressingHashTable()
	{
		_clear();
	}
	
	
	void insert(const Key& key, const Value& value)
	{
		size_t index = 0;
		
		auto hash = _linear_hash(key, index);
		
		for ( ; _nodes[hash]; hash = _linear_hash(key, ++index))
		{
			if (_nodes[hash]->key == key)
			{
				_nodes[hash]->value = value;
				
				return;
			}
		}
		
		_nodes[hash] = new Node(key, value);
		
		if (++_size == _capacity/2)
		{
			resize(_capacity * 2);
		}
	}
	
	
	Value& get(const Key& key)
	{
		return _get(key);
	}
	
	const Value& get(const Key& key) const
	{
		return _get(key);
	}
	
	bool contains(const Key& key) const
	{
		size_t index = 0;
		
		for (auto hash = _linear_hash(key, index);
			 _nodes[hash];
			 hash = _linear_hash(key, ++index))
		{
			if (_nodes[hash]->is_alive && _nodes[hash]->key == key)
			{
				return true;
			}
		}
		
		return false;
	}
	
	Value& operator[](const Key& key)
	{
		size_t index = 0;
		
		auto hash = _linear_hash(key, index);
		
		for ( ;_nodes[hash]; hash = _linear_hash(key, ++index))
		{
			if (_nodes[hash]->is_alive && _nodes[hash]->key == key)
			{
				return _nodes[hash]->value;
			}
		}
		
		_nodes[hash] = new Node(key);
		
		if (++_size == _capacity/2)
		{
			resize(_capacity * 2);
		}
		
		return _nodes[hash]->value;
	}
	
	
	void erase(const Key& key)
	{
		size_t index = 0;
		
		for (auto hash = _linear_hash(key, index);
			 _nodes[hash];
			 hash = _linear_hash(key, ++index))
		{
			if (_nodes[hash]->is_alive && _nodes[hash]->key == key)
			{
				_nodes[hash]->is_alive = false;
				
				if (--_size == _capacity/8)
				{
					resize(_capacity/2);
				}
				
				return;
			}
		}
		
		throw std::invalid_argument("No such key!");
	}
	
	void clear()
	{
		_clear();
		
		_capacity = minimum_capacity;
		
		_nodes = new Node*[_capacity];
		
		_size = 0;
	}
	
	
	size_t size() const noexcept
	{
		return _size;
	}
	
	bool is_empty() const noexcept
	{
		return _size == 0;
	}
	
	
	void pre_hash(const pre_hash_t& pre_hash)
	{
		_pre_hash = pre_hash;
	}
	
	const pre_hash_t& pre_hash() const noexcept
	{
		return _pre_hash;
	}
	
	
	void resize(size_t new_size)
	{
		auto old = _nodes;
		
		auto old_capacity = _capacity;
		
		_size = new_size;
		
		_capacity = new_size * 2;
		
		_nodes = new Node*[_capacity];
		
		std::fill(_nodes, _nodes + _capacity, nullptr);
		
		_rehash(old, old_capacity);
		
		delete [] old;
	}
	
	void rehash()
	{
		auto old = _nodes;
		
		_nodes = new Node*[_capacity];
		
		std::fill(_nodes, _nodes + _capacity, nullptr);
		
		_rehash(old, _capacity);
	}
	
private:
	
	struct Node
	{
		Node(const Key& key_,
			 const Value& value_ = Value(),
			 bool is_alive_ = true)
		: key(key_)
		, value(value_)
		, is_alive(is_alive_)
		{ }
		
		Key key;
		
		Value value;
		
		bool is_alive;
	};
	
	void _rehash(Node** old, size_t old_capacity)
	{
		for (size_t i = 0; i < old_capacity; ++i)
		{
			if (old[i] && old[i]->is_alive)
			{
				auto probe_index = 0;
				
				auto new_hash = _linear_hash(old[i]->key, probe_index);
				
				while (_nodes[new_hash])
				{
					new_hash = _linear_hash(old[i]->key, ++probe_index);
				}
				
				_nodes[new_hash] = old[i];
			}
		}
	}
	
	void _clear()
	{
		for (size_t index = 0; index < _capacity; ++index)
		{
			if (_nodes[index]) delete _nodes[index];
		}
		
		delete [] _nodes;
	}
	
	Value& _get(const Key& key) const
	{
		size_t index = 0;
		
		for (auto hash = _linear_hash(key, index);
			 _nodes[hash];
			 hash = _linear_hash(key, ++index))
		{
			if (_nodes[hash]->is_alive && key == _nodes[hash]->key)
			{
				return _nodes[hash]->value;
			}
		}
		
		throw std::invalid_argument("No such key!");
	}
	
	// Linear probing
	size_t _linear_hash(const Key& key, size_t index) const
	{
		return (_pre_hash(key) + index) % _capacity;
	}
	
	size_t _quadratic_hash(const Key& key, size_t index) const
	{
		static const size_t c_1 = 69;
		static const size_t c_2 = 123;
		
		auto hash = _pre_hash(key) + c_1 * index + c_2 * (index * index);
		
		return hash % _capacity;
	}
	
	size_t _double_hash(const Key& key, size_t index)
	{
		auto increment = index * _second_pre_hash(key);
		
		if (index && increment % 2 == 0) ++increment;
		
		auto hash = _first_pre_hash(key) + increment;
		
		return hash % _capacity;
	}
	
	size_t _size;
	
	size_t _capacity;
	
	pre_hash_t _pre_hash;
	
	Node** _nodes;
};

#endif /* OPEN_ADDRESSING_HASH_TABLE_HPP */