#ifndef CUCKOO_HASH_TABLE_HPP
#define CUCKOO_HASH_TABLE_HPP

template<typename Key, typename Value>
class Table
{
public:
	
	using size_t = std::size_t;
	
	struct Item
	{
		using hashes_t = std::pair<size_t, size_t>;
		
		Item(const hashes_t& h = hashes_t(),
			 const Key& k = Key(),
			 const Value& v = Value())
		: key(k)
		, value(v)
		, hashes(h)
		{ }
		
		Key key;
		Value value;
		
		hashes_t hashes;
	};
	
	class Data
	{
	public:
		
		using item_ptr = Item*;
		
		using data_t = item_ptr*;
		
		using iterator = const data_t;
		
		Data()
		: _size(0)
		, _items(nullptr)
		{ }
		
		Data(size_t size, size_t extra = 0)
		: _size(size)
		, _extra(extra)
		, _items(new item_ptr[_size + extra])
		{ }
		
		Data(const Data& other)
		: _size(other._size)
		, _extra(other._extra)
		, _items(new item_ptr[_size])
		{
			std::copy(other.begin(), other.end(), begin());
		}
		
		Data(Data&& other) noexcept
		: Data()
		{
			swap(other);
		}
		
		Data& operator=(Data other)
		{
			swap(other);
			
			return *this;
		}
		
		void swap(Data& other) noexcept
		{
			using std::swap;
			
			swap(_size, other._size);
			
			swap(_extra, other._extra);
			
			swap(_items, other._items);
		}
		
		friend void swap(Data& first, Data& second)
		{
			first.swap(second);
		}
		
		~Data()
		{
			delete _items;
		}
		
		item_ptr& operator[](size_t index) const
		{
			return _items[index];
		}
		
		iterator begin() const
		{
			return _items;
		}
		
		iterator end() const
		{
			return _items + _size;
		}
		
		size_t size() const
		{
			return _size;
		}
		
		void clear()
		{
			for (auto& i : *this) delete i;
			
			for (size_t i = 0; i < _extra; ++i)
			{
				delete _items[_size + i];
			}
		}
		
	private:
		
		size_t _size;
		
		size_t _extra;
		
		data_t _items;
	};
	
	using item_ptr = typename Data::item_ptr;
	
	using iterator = typename Data::iterator;
	
	using constants_t = std::array<size_t, 3>;
	
	
	enum Constants { A, B, PRIME };
	
	
	Table() = default;
	
	Table(size_t table_size)
	: _data(table_size)
	{
		generate_constants();
		nullify();
	}
	
	Table(const Table& other)
	: _data(other._data)
	, _constants(other._constants)
	{ }
	
	Table(Table&& other) noexcept
	: Table()
	{
		swap(other);
	}
	
	Table& operator=(Table other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(Table& other) noexcept
	{
		using std::swap;
		
		swap(_data, other._data);
		
		swap(_constants, other._constants);
	}
	
	friend void swap(Table& first, Table& second) noexcept
	{
		first.swap(second);
	}
	
	~Table()
	{
		clear();
	}
	
	iterator begin() const
	{
		return _data.begin();
	}
	
	iterator end() const
	{
		return _data.end();
	}
	
	item_ptr& front() const
	{
		return _data[0];
	}
	
	item_ptr& back() const
	{
		return _data[size() - 1];
	}
	
	item_ptr& operator[](size_t index) const
	{
		return _data[index];
	}
	
	item_ptr& extra(size_t index = 0) const
	{
		return _data[size() + index];
	}
	
	size_t hash(size_t pre_hash) const
	{
		size_t result = _constants[A] * pre_hash + _constants[B];
		
		return (result % _constants[PRIME]) % _data.size();
	}
	
	void generate_constants()
	{
		using distribution_t = std::uniform_int_distribution<size_t>;
		
		static std::random_device seed;
		static std::mt19937 generator(seed());
		static const size_t bit_width = sizeof(size_t) * 8;
		
		distribution_t distribution(bit_width, 1E6);
		
		_constants[A] = distribution(generator);
		_constants[B] = distribution(generator);
		
		do
		{
			_constants[PRIME] = distribution(generator);
			
		} while (! _is_prime(_constants[PRIME]));
	}
	
	const Data& items() const
	{
		return _data;
	}
	
	void reset(size_t new_size, size_t extra = 0)
	{
		_data = Data(new_size, extra);
		
		nullify();
	}
	
	void nullify()
	{
		std::fill(begin(), end(), nullptr);
	}
	
	void clear()
	{
		_data.clear();
	}
	
	size_t size() const
	{
		return _data.size();
	}
	
private:
	
	static bool _is_prime(size_t value)
	{
		if (value <= 1) return false;
		
		if (value <= 3) return true;
		
		if (value % 2 == 0 || value % 3 == 0) return false;
		
		const size_t boundary = std::sqrt(value);
		
		for (size_t prime = 5; prime <= boundary; prime += 6)
		{
			if (value % prime == 0 || value % (prime + 2) == 0)
			{
				return false;
			}
		}
		
		return true;
	}
	
	
	Data _data;
	
	constants_t _constants;
};

template<typename Key, typename Value>
class CuckooHashMap
{
public:
	
	struct Pair
	{
		Pair(const Key& k, Value& v)
		: key(k)
		, value(v)
		{ }
		
		const Key& key;
		Value& value;
	};
	
private:
	
	static const size_t CYCLE_LIMIT = 16;
	
	enum Index { FIRST, SECOND };
	
	
	using table_t = Table<Key, Value>;
	
	using container_t = std::array<table_t, 2>;
	
	using item_t = typename table_t::Item;
	
	using item_ptr = item_t*;
	
	
	class BaseIterator
	{
	public:
		
		BaseIterator()
		: _tables(nullptr)
		, _item(nullptr)
		, _pair(nullptr)
		{}
		
		BaseIterator(const container_t& tables, item_ptr& item)
		: _tables(&tables)
		, _item(&item)
		, _pair(nullptr)
		{ }
		
		BaseIterator(const container_t& tables)
		: _tables(&tables)
		, _item(&tables[FIRST].front())
		, _pair(nullptr)
		{
			--_item;
			
			// Find the next valid pointer; just sets back
			// to the one passed if it was good
			++*this;
		}
		
		virtual ~BaseIterator()
		{
			delete _pair;
		}
		
		virtual BaseIterator& operator++()
		{
			do
			{
				if (++_item == (*_tables)[FIRST].end())
				{
					_item = (*_tables)[SECOND].begin();
				}
			}
			
			while (! *_item && _item < (*_tables)[SECOND].end());
			
			_clear_pair();
			
			return *this;
		}
		
		virtual BaseIterator operator++(int)
		{
			BaseIterator previous = *this;
			
			++*this;
			
			return previous;
		}
		
		virtual BaseIterator& operator--()
		{
			do
			{
				if (_item == (*_tables)[SECOND].begin())
				{
					_item = (*_tables)[FIRST].end() - 1;
				}
				
				else --_item;
			}
			
			while (! *_item && _item >= (*_tables)[FIRST].begin());
			
			_clear_pair();
			
			return *this;
		}
		
		virtual BaseIterator operator--(int)
		{
			BaseIterator following = *this;
			
			--*this;
			
			return following;
		}
		
		virtual bool operator==(const BaseIterator& other) const
		{
			return _item == other._item;
		}
		
		virtual bool operator!=(const BaseIterator& other) const
		{
			return _item != other._item;
		}
		
		
	protected:
		
		void _check_pair()
		{
			if (! _pair)
			{
				_pair = new Pair((*_item)->key, (*_item)->value);
			}
		}
		
		void _clear_pair()
		{
			if (_pair)
			{
				delete _pair;
				
				_pair = nullptr;
			}
		}
		
		const container_t* _tables;
		
		item_ptr* _item;
		
		Pair* _pair;
	};
	
public:
	
	using size_t = std::size_t;
	
	using pre_hash_t = std::function<size_t(const Key&)>;
	
	static const size_t MINIMUM_CAPACITY = 16;
	
	
	struct ConstIterator : public BaseIterator
	{
		
		using BaseIterator::_check_pair;
		using BaseIterator::_item;
		using BaseIterator::_pair;
		
		ConstIterator() = default;
		
		ConstIterator(const container_t& tables, item_ptr& item)
		: BaseIterator(tables, item)
		{ }
		
		ConstIterator(const container_t& tables)
		: BaseIterator(tables)
		{ }
		
		const Pair& operator*() const
		{
			_check_pair();
			
			return *_pair;
		}
		
		const Pair* operator->() const
		{
			_check_pair();
			
			return _pair;
		}
	};
	
	struct Iterator : public BaseIterator
	{
		using BaseIterator::_check_pair;
		using BaseIterator::_pair;
		using BaseIterator::_tables;
		using BaseIterator::_item;
		
		Iterator() = default;
		
		Iterator(const container_t& tables, item_ptr& item)
		: BaseIterator(tables, item)
		{ }
		
		Iterator(const container_t& tables)
		: BaseIterator(tables)
		{ }
		
		Pair& operator*()
		{
			_check_pair();
			
			return *_pair;
		}
		
		Pair* operator->()
		{
			_check_pair();
			
			return _pair;
		}
		
		operator ConstIterator() const
		{
			return {_tables, _item};
		}
	};
	
	
	CuckooHashMap(const pre_hash_t& pre_hash = std::hash<Key>(),
				  size_t capacity = MINIMUM_CAPACITY)
	: _size(0)
	, _capacity(capacity)
	, _pre_hash(pre_hash)
	, _tables({
		_capacity/2,
		_capacity/2 + 1
	})
	{
		_tables[SECOND].extra() = new item_t;
	}
	
	CuckooHashMap(std::initializer_list<std::pair<Key, Value>> items,
				  const pre_hash_t& pre_hash = std::hash<Key>(),
				  size_t capacity = MINIMUM_CAPACITY)
	: _size(0)
	, _capacity(std::max(items.size(), capacity))
	, _pre_hash(pre_hash)
	, _tables({
		_capacity/2,
		_capacity/2 + 1
	})
	{ }
	
	CuckooHashMap(const CuckooHashMap& other)
	: _size(other._size)
	, _capacity(other._capacity)
	, _pre_hash(other._pre_hash)
	, _tables(other._tables)
	{ }
	
	CuckooHashMap(CuckooHashMap&& other) noexcept
	: CuckooHashMap()
	{
		swap(other);
	}
	
	CuckooHashMap& operator=(CuckooHashMap other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(CuckooHashMap& other) noexcept
	{
		using std::swap;
		
		swap(_size, other._size);
		
		swap(_capacity, other._capacity);
		
		swap(_tables, other._tables);
		
		swap(_pre_hash, other._pre_hash);
	}
	
	friend void swap(CuckooHashMap& first, CuckooHashMap& second)
	{
		first.swap(second);
	}
	
	~CuckooHashMap() = default;
	
	
	Iterator begin()
	{
		return {_tables};
	}
	
	Iterator end()
	{
		return _iterator(_tables[SECOND].extra());
	}
	
	ConstIterator begin() const
	{
		return {_tables};
	}
	
	ConstIterator end() const
	{
		return _iterator(_tables[SECOND].extra());
	}
	
	Iterator insert(const Key& key, const Value& value)
	{
		auto update = _try_update(key, value);
		
		if (update.first != end()) return update.first;
		
		auto p_item = new item_t(update.second, key, value);
		
		return _insert(p_item);
	}
	
	Iterator insert(const Pair& pair)
	{
		return insert(pair.key, pair.value);
	}
	
	template<typename Itr>
	void insert(Itr begin, Itr end)
	{
		for ( ; begin != end; ++begin)
		{
			insert(*begin);
		}
	}
	
	void erase(Iterator itr)
	{
		erase(itr->key);
	}
	
	template<typename Itr>
	void erase(Itr begin, Itr end)
	{
		for ( ; begin != end; ++begin)
		{
			erase(*begin);
		}
	}
	
	void erase(const Key& key)
	{
		if (! erase_if_found(key))
		{
			throw std::invalid_argument("No such key!");
		}
	}
	
	bool erase_if_found(const Key& key)
	{
		auto hashes = _hashes(key);
		
		auto& first = _first(hashes.first);
		
		if (first && first->key == key)
		{
			_erase(first);
			
			return true;
		}
		
		auto& second = _second(hashes.second);
		
		if (second && second->key == key)
		{
			_erase(second);
			
			return true;
		}
		
		return false;
		
	}
	
	void clear()
	{
		_capacity = MINIMUM_CAPACITY;
		
		for (auto& table : _tables) table.clear();
		
		size_t table_size = _capacity/2;
		
		_tables[FIRST].reset(table_size);
		_tables[SECOND].reset(table_size, 1);
		
		_tables[SECOND].extra() = new item_t;
		
		_size = 0;
	}
	
	
	Value& at(const Key& key)
	{
		return _at(key);
	}
	
	const Value& at(const Key& key) const
	{
		return _at(key);
	}
	
	
	bool contains(const Key& key) const
	{
		auto hashes = _hashes(key);
		
		auto first = _first(hashes.first);
		
		if (first && first->key == key)
		{
			return true;
		}
		
		auto second = _second(hashes.second);
		
		if (second && second->key == key)
		{
			return true;
		}
		
		return false;
	}
	
	Iterator find(const Key& key)
	{
		return _find(key);
	}
	
	ConstIterator find(const Key& key) const
	{
		return _find(key);
	}
	
	
	Value& operator[](const Key& key)
	{
		try
		{
			return _at(key);
		}
		
		catch(std::invalid_argument&)
		{
			auto p_item = new item_t(_hashes(key), key);
			
			_insert(p_item);
			
			return p_item->value;
		}
	}
	
	std::pair<Iterator, bool> insert_or_assign(const Key& key, Value&& value)
	{
		try
		{
			_at(key) = std::forward<Value>(value);
			
			return {end(), false};
		}
		
		catch(std::invalid_argument&)
		{
			auto iterator = _insert(new item_t(_hashes(key), key, value));
			
			return {iterator, true};
		}
	}
	
	
	size_t size() const
	{
		return _size;
	}
	
	size_t capacity() const
	{
		return _capacity;
	}
	
	bool is_empty() const
	{
		return _size == 0;
	}
	
	const pre_hash_t& pre_hash() const
	{
		return _pre_hash;
	}
	
	
private:
	
	using hashes_t = std::pair<size_t, size_t>;
	
	using update_t = std::pair<Iterator, hashes_t>;
	
	using data_t = std::array<typename table_t::Data, 2>;
	
	
	Iterator _iterator(item_ptr& p_item)
	{
		return {_tables, p_item};
	}
	
	ConstIterator _iterator(item_ptr& p_item) const
	{
		return {_tables, p_item};
	}
	
	Iterator _find(const Key& key) const
	{
		auto hashes = _hashes(key);
		
		auto first = _first(hashes.first);
		
		if (first && first->key == key)
		{
			return _iterator(first);
		}
		
		auto second = _second(hashes.second);
		
		if (second && second->key == key)
		{
			return _iterator(second);
		}
		
		return end();
	}
	
	Iterator _insert(item_ptr p_item)
	{
		size_t iterations = 0;
		
		auto& first = _first(p_item);
		
		std::swap(p_item, first);
		
		auto iterator = _iterator(first);
		
		while (p_item)
		{
			if (++iterations > CYCLE_LIMIT)
			{
				_rehash(_items());
				
				break;
			}
			
			std::swap(p_item, _second(p_item));
			
			if (! p_item) break;
			
			std::swap(p_item, _first(p_item));
		}
		
		if (++_size == _capacity/2) _resize();
		
		return iterator;
	}
	
	void _erase(item_ptr& p_item)
	{
		delete p_item;
		
		p_item = nullptr;
		
		if (--_size == _capacity/8) _resize();
	}
	
	Value& _at(const Key& key) const
	{
		auto hashes = _hashes(key);
		
		auto& first = _first(hashes.first);
		
		if (first && first->key == key)
		{
			return first->value;
		}
		
		auto& second = _second(hashes.second);
		
		if (second && second->key == key)
		{
			return second->value;
		}
		
		throw std::invalid_argument("No such key!");
	}
	
	update_t _try_update(const Key& key, const Value& value)
	{
		auto hashes = _hashes(key);
		
		auto& first = _first(hashes.first);
		
		if (first && first->key == key)
		{
			first->value = value;
			
			return {_iterator(first), hashes};
		}
		
		auto& second = _second(hashes.second);
		
		if (second && second->key == key)
		{
			second->value = value;
			
			return {_iterator(second), hashes};
		}
		
		return {end(), hashes};
	}
	
	hashes_t _hashes(const Key& key) const
	{
		size_t pre_hash = _pre_hash(key);
		
		size_t hash_1 = _tables[FIRST].hash(pre_hash);
		size_t hash_2 = _tables[SECOND].hash(pre_hash);
		
		return {hash_1, hash_2};
	}
	
	inline item_ptr& _first(const item_ptr item) const
	{
		return _first(item->hashes.first);
	}
	
	inline item_ptr& _second(const item_ptr item) const
	{
		return _second(item->hashes.second);
	}
	
	inline item_ptr& _first(size_t hash) const
	{
		return _tables[FIRST][hash];
	}
	
	inline item_ptr& _second(size_t hash) const
	{
		return _tables[SECOND][hash];
	}
	
	data_t _items()
	{
		auto& first = _tables[FIRST].items();
		auto& second = _tables[SECOND].items();
		
		return {first, second};
	}
	
	void _resize()
	{
		size_t new_capacity = _size * 4;
		
		if (new_capacity < MINIMUM_CAPACITY) return;
		
		size_t old_capacity = _capacity;
		
		_capacity = new_capacity;
		
		auto old = _items();
		
		_tables[FIRST].reset(_capacity);
		_tables[SECOND].reset(_capacity, 1);
		
		_rehash(std::move(old), old_capacity);
		
		_tables[SECOND].extra() = new item_t;
	}
	
	void _rehash(data_t old, size_t old_capacity)
	{
		size_t old_table_size = old_capacity/2;
		
		do
		{
			for (auto& table : _tables)
			{
				table.nullify();
				table.generate_constants();
			}
			
		} while (! _try_rehash(old, old_table_size));
	}
	
	bool _try_rehash(data_t old, size_t old_table_size)
	{
		for (auto& table : old)
		{
			for (auto& p_item : table)
			{
				if (! p_item) continue;
				
				p_item->hashes = _hashes(p_item->key);
				
				size_t iterations = 0;
				
				do
				{
					if (++iterations > CYCLE_LIMIT) return false;
					
					std::swap(p_item, _first(p_item));
					
					if (! p_item) break;
					
					std::swap(p_item, _second(p_item));
					
				} while(p_item);
			}
		}
		
		return true;
	}
	
	void _rehash(data_t old)
	{
		_rehash(std::move(old), _capacity);
	}
	
	
	size_t _size;
	size_t _capacity;
	
	container_t _tables;
	
	pre_hash_t _pre_hash;
};

#endif /* CUCKOO_HASH_TABLE_HPP */