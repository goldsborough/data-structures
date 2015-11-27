#ifndef SEPARATE_CHAINING_HASH_TABLE_HPP
#define SEPARATE_CHAINING_HASH_TABLE_HPP

#include <algorithm>
#include <cmath>
#include <functional>

template<typename Key, typename Value>
class SeparateChainingHashTable
{
public:
	
	using size_t = std::size_t;
	
	using pre_hash_t = std::function<size_t(const Key& key)>;
	
	static const size_t minimum_capacity;
	
	SeparateChainingHashTable(const pre_hash_t& pre_hash = std::hash<Key>(),
							  size_t load_factor = 4,
							  size_t capacity = minimum_capacity)
	: _size(0)
	, _threshold(capacity)
	, _pre_hash(pre_hash)
	, _load_factor(load_factor)
	, _capacity(_threshold/load_factor)
	, _nodes(new Node*[_capacity])
	{
		std::fill(_nodes, _nodes + _capacity, nullptr);
	}
	
	SeparateChainingHashTable(std::initializer_list<std::pair<Key, Value>> list,
							  const pre_hash_t& pre_hash = std::hash<Key>(),
							  size_t load_factor = 4,
							  size_t capacity = minimum_capacity)
	: _size(0)
	, _threshold(std::max(capacity, list.size()))
	, _pre_hash(pre_hash)
	, _load_factor(load_factor)
	, _capacity(_threshold/load_factor)
	, _nodes(new Node*[_capacity])
	{
		std::fill(_nodes, _nodes + _capacity, nullptr);
		
		for (const auto& item : list)
		{
			insert(item.first, item.second);
		}
	}
	
	SeparateChainingHashTable(const SeparateChainingHashTable& other)
	: _size(other._size)
	, _capacity(other._capacity)
	, _threshold(other._threshold)
	, _pre_hash(other._pre_hash)
	, _load_factor(other._load_factor)
	, _nodes(new Node*[_capacity])
	{
		std::fill(_nodes, _nodes + _capacity, nullptr);
		
		for (size_t i = 0; i < other._capacity; ++i)
		{
			for (auto node = other._nodes[i]; node; node = node->next)
			{
				insert(node->key, node->value);
			}
		}
	}
	
	SeparateChainingHashTable(SeparateChainingHashTable&& other) noexcept
	: SeparateChainingHashTable()
	{
		swap(other);
	}
	
	SeparateChainingHashTable& operator=(SeparateChainingHashTable other)
	{
		swap(other);
		
		return *this;
	}

	void swap(SeparateChainingHashTable& other) noexcept
	{
		using std::swap;
		
		swap(_nodes, other._nodes);
		
		swap(_capacity, other._capacity);
		
		swap(_size, other._size);
		
		swap(_pre_hash, other._pre_hash);
		
		swap(_load_factor, other._load_factor);
	}
	
	friend void swap(SeparateChainingHashTable& first,
					 SeparateChainingHashTable& second) noexcept
	{
		first.swap(second);
	}
	
	~SeparateChainingHashTable()
	{
		_clear();
	}
	
	
	void insert(const Key& key, const Value& value)
	{
		auto index = _hash(key);
		
		for (auto node = _nodes[index]; node; node = node->next)
		{
			if (node->key == key)
			{
				node->value = value;
				
				return;
			}
		}
		
		auto node = new Node(key, value, _nodes[index]);
		
		_nodes[index] = node;
		
		if (++_size == _threshold) resize(_size * 2);
	}
	
	
	void erase(const Key& key)
	{
		auto index = _hash(key);
		
		Node* previous = nullptr;
		
		for (auto node = _nodes[index];
			 node;
			 previous = node, node = node->next)
		{
			if (node->key == key)
			{
				if (previous) previous->next = node->next;
				
				else _nodes[index] = node->next;
				
				delete node;
				
				if (--_size == _threshold/4) resize(_size * 2);
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
	
	Value& get(const Key& key)
	{
		return _get(key);
	}
	
	const Value& get(const Key& key) const
	{
		return _get(key);
	}
	
	
	bool contains(const Key& key)
	{
		auto index = _hash(key);
		
		for (auto node = _nodes[index]; node; node = node->next)
		{
			if (node->key == key) return true;
		}
		
		return false;
	}
	
	
	Value& operator[](const Key& key)
	{
		auto index = _hash(key);
		
		for (auto node = _nodes[index]; node; node = node->next)
		{
			if (node->key == key) return node->value;
		}
		
		auto node = new Node(key, Value(), _nodes[index]);
		
		_nodes[index] = node;
		
		if (++_size == _threshold) resize(_size * 2);
			
		return node->value;
	}
	
	
	size_t size() const
	{
		return _size;
	}
	
	bool is_empty() const
	{
		return _size == 0;
	}
	
	
	size_t load_factor() const
	{
		return _load_factor;
	}
	
	void load_factor(size_t alpha)
	{
		_load_factor = alpha;
		
		rehash();
	}
	
	
	const pre_hash_t& pre_hash() const
	{
		return _pre_hash;
	}
	
	void pre_hash(const pre_hash_t& pre_hash)
	{
		_pre_hash = pre_hash;
		
		rehash();
	}
	
	
	void resize(size_t size)
	{
		size = std::max(size, minimum_capacity/2);
		
		auto old = _nodes;
		
		auto old_capacity = _capacity;
		
		_threshold = size * 2;
		
		_capacity = _threshold / _load_factor;
		
		_size = size;
		
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
		
		delete [] old;
	}
	
private:
	
	struct Node
	{
		Node(const Key& key_,
			 const Value& value_ = Value(),
			 Node* next_ = nullptr)
		: key(key_)
		, value(value_)
		, next(next_)
		{ }
		
		
		Key key;
		
		Value value;
		
		Node* next;
	};
	
	void _clear()
	{
		for (size_t i = 0; i < _capacity; ++i)
		{
			for (auto node = _nodes[i]; node; )
			{
				auto next = node->next;
				
				delete node;
				
				node = next;
			}
		}
	}
	
	void _rehash(Node** old, size_t old_capacity)
	{
		for (size_t i = 0; i < old_capacity; ++i)
		{
			for (auto node = _nodes[i]; node; )
			{
				auto new_index = _hash(node->key);
				
				auto next = node->next;
				
				node->next = _nodes[new_index];
				
				_nodes[new_index] = node;
				
				node = next;
			}
		}
	}
	
	Value& _get(const Key& key) const
	{
		auto index = _hash(key);
		
		for (auto node = _nodes[index]; node; node = node->next)
		{
			if (node->key == key) return node->value;
		}
		
		throw std::invalid_argument("No such key!");
	}
	
	size_t _hash3(const Key& key) const
	{
		static const size_t a = 99;
		static const size_t b = 123;
		static const size_t p = 69;
		
		return (((key * a) + b) % p) % _capacity;
	}
	
	size_t _hash2(const Key& key) const
	{
		static const size_t constant = 99;
		
		static const size_t word_size = sizeof(size_t) * 8;
		
		static const size_t word_max = 1 << (word_size - 1);
		
		const size_t wanted = word_size - std::log2(_capacity);
		
		// ((key * c) % 2^w) >> (w - r);
		return ((_pre_hash(key) * constant) % word_max) >> wanted;
	}
	
	size_t _hash(const Key& key) const
	{
		return _pre_hash(key) % _capacity;
	}
	
	
	size_t _size;
	
	size_t _threshold;
	
	size_t _capacity;
	
	size_t _load_factor;
	
	pre_hash_t _pre_hash;
	
	Node** _nodes;
};

template<typename Key, typename Value>
const typename SeparateChainingHashTable<Key, Value>::size_t
SeparateChainingHashTable<Key, Value>::minimum_capacity = 16;


#endif /* SEPARATE_CHAINING_HASH_TABLE_HPP */