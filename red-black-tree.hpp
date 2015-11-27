#ifndef RED_BLACK_TREE_HPP
#define RED_BLACK_TREE_HPP

#include <assert.h>
#include <initializer_list>
#include <stdexcept>

template<typename Key, typename Value>
class RedBlackTree
{
public:
	
	using size_t = std::size_t;
	
	RedBlackTree()
	: _size(0)
	, _root(nullptr)
	{ }
	
	RedBlackTree(std::initializer_list<std::pair<Key, Value>> list)
	: RedBlackTree()
	{
		for (const auto& item : list)
		{
			insert(item.first, item.second);
		}
	}
	
	RedBlackTree(const RedBlackTree& other)
	: _size(other._size)
	, _root(_copy(other._root))
	{ }
	
	RedBlackTree(RedBlackTree&& other) noexcept
	: RedBlackTree()
	{
		swap(other);
	}
	
	RedBlackTree& operator=(RedBlackTree other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(RedBlackTree& other) noexcept
	{
		// Enable ADL
		using std::swap;
		
		
		swap(_root, other._root);
		
		swap(_size, other._size);
	}
	
	friend void swap(RedBlackTree& first, RedBlackTree& second) noexcept
	{
		first.swap(second);
	}
	
	~RedBlackTree()
	{
		_clear(_root);
	}
	
	
	void insert(const Key& key, const Value& value)
	{
		_root = _insert(_root, key, value);
	}
	
	
	Value& get(const Key& key)
	{
		auto node = _find(_root, key);
		
		if (! node)
		{
			throw std::invalid_argument("No such key!");
		}
		
		return node->value;
	}
	
	const Value& get(const Key& key) const
	{
		auto node = _find(_root, key);
		
		if (! node)
		{
			throw std::invalid_argument("No such key!");
		}
		
		return node->value;
	}
	
	
	bool contains(const Key& key)
	{
		auto node = _find(_root, key);
		
		return node != nullptr;
	}
	
	
	Value& operator[](const Key& key)
	{
		auto node = _find(_root, key);
		
		if (! node)
		{
			node = new Node(key);
			
			_root = _insert(_root, node);
		}
		
		return node->value;
	}
	
	
	void erase(const Key& key)
	{
		_root = _erase(_root, key);
	}
	
	void clear()
	{
		_clear(_root);
		
		_size = 0;
		
		_root = nullptr;
	}
	
	
	size_t rank(const Key& key) const
	{
		return _rank(_root, key);
	}
	
	const Key& select(size_t rank) const
	{
		if (rank == 0)
		{
			throw std::invalid_argument("No key of such rank!");
		}
		
		auto node = _select(_root, rank);
		
		if (! node)
		{
			throw std::invalid_argument("No key of such rank!");
		}
		
		return node->key;
	}
	
	const Key& ceiling(const Key& key) const
	{
		auto node = _ceiling(_root, key);
		
		if (! node)
		{
			throw std::invalid_argument("No ceiling for given key!");
		}
		
		return node->key;
	}
	
	const Key& floor(const Key& key) const
	{
		auto node = _floor(_root, key);
		
		if (! node)
		{
			throw std::invalid_argument("No floor for given key!");
		}
		
		return node->key;
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
	
	enum class Color { Red, Black };
	
	struct Node
	{
		Node(const Key& key_,
			 const Value& value_ = Value(),
			 Node* left_ = nullptr,
			 Node* right_ = nullptr)
		: key(key_)
		, value(value_)
		, left(left_)
		, right(right_)
		, size(0)
		, color(Color::Red)
		{ }
		
		
		void resize()
		{
			size = 1;
			
			if (left) size += left->size;
			
			if (right) size += right->size;
		}
		
		
		Key key;
		
		Value value;
		
		
		Node* left;
		
		Node* right;
		
		
		size_t size;
		
		Color color;
	};
	
	Node* _select(Node* node, size_t rank)
	{
		if (! node) return node;
		
		size_t size = 1;
		
		if (node->left) rank += node->left->size;
	
		if (size < rank)
		{
			return _select(node->right, rank - size);
		}
		
		else if (size > rank)
		{
			return _select(node->left, rank);
		}
		
		else return node;
	}
	
	size_t _rank(Node* node, const Key& key)
	{
		if (! node) return 0;
		
		if (key < node->key) return _rank(node->left, key);
		
		size_t rank = 1;
		
		if (node->left) rank += node->left->size;
		
		if (node->right) rank += _rank(node->right, key);
		
		return rank;
	}
	
	Node* _ceiling(Node* node, const Key& key) const
	{
		if (! node) return nullptr;
		
		if (key < node->key)
		{
			auto result = _ceiling(node->left, key);
			
			return result ? result : node;
		}
		
		else return _ceiling(node->right, key);
	}
	
	const Key& _floor(Node* node, const Key& key) const
	{
		if (! node) return node;
		
		if (key > node->key)
		{
			auto result = _floor(node->right, key);
			
			return result ? result : node;
		}
		
		else return _floor(node->left, key);
	}
	
	Node* _rotate_left(Node* node)
	{
		assert(_is_red(node->right));
		
		
		auto right = node->right;
		
		node->right = right->left;
		
		right->left = node;
		
		
		right->color = node->color;
		
		node->color = Color::Red;
		
		
		node->resize();
		
		return right;
	}
	
	Node* _rotate_right(Node* node)
	{
		assert(_is_red(node->left));
		
		
		auto left = node->left;
		
		node->left = left->right;
		
		left->right = node;
		
		
		left->color = node->color;
		
		node->color = Color::Red;
		
		
		node->resize();
		
		return left;
	}
	
	void _color_flip(Node* node)
	{
		assert(node);
		assert(_is_red(node->left));
		assert(_is_red(node->right));
		
		node->color = Color::Red;
		
		node->left->color = Color::Black;
		node->right->color = Color::Black;
	}
	
	Node* _handle_colors(Node* node)
	{
		if (_is_red(node->right))
		{
			node = _rotate_left(node);
		}
		
		if (_is_red(node->left) && _is_red(node->left->left))
		{
			node = _rotate_right(node);
		}
		
		if (_is_red(node->left) && _is_red(node->right))
		{
			_color_flip(node);
		}
		
		node->resize();
		
		return node;
	}
	
	bool _is_red(Node* node)
	{
		return node && node->color == Color::Red;
	}
	
	void _clear(Node* node)
	{
		if (! node) return;
		
		if (node->left) _clear(node->left);
		
		if (node->right) _clear(node->right);
		
		delete node;
	}
	
	
	Node* _insert(Node* node, const Key& key, const Value& value)
	{
		if (! node)
		{
			++_size;
			
			return new Node(key, value);
		}
		
		if (key < node->key)
		{
			node->left = _insert(node->left, key, value);
		}
		
		else if (key > node->key)
		{
			node->right = _insert(node->right, key, value);
		}
		
		else node->value = value;
		
		return _handle_colors(node);
	}
	
	Node* _insert(Node* node, Node* new_node)
	{
		if (! node)
		{
			++_size;
			
			return new_node;
		}
		
		if (new_node->key < node->key)
		{
			node->left = _insert(node->left, new_node);
		}
		
		else if (new_node->key > node->key)
		{
			node->right = _insert(node->right, new_node);
		}
		
		else
		{
			new_node->left = node->left;
			new_node->right = node->right;
			
			delete node;
			
			node = new_node;
		}
		
		return _handle_colors(node);
	}
	
	Node* _find(Node* node, const Key& key) const
	{
		if (! node) return nullptr;
		
		if (key < node->key) return _find(node->left, key);
		
		else if (key > node->key) return _find(node->right, key);
		
		else return node;
	}
	
	Node* _erase(Node* node, const Key& key)
	{
		if (! node) throw std::invalid_argument("No such key!");
		
		if (key < node->key) node->left = _erase(node->left, key);
		
		else if (key > node->key) node->right = _erase(node->right, key);
		
		else node = get_successor(node);
		
		return _handle_colors(node);
	}
	
	Node* _get_successor(Node* node)
	{
		if (! node->left)
		{
			auto right = node->right;
			
			delete node;
			
			--_size;
			
			return right;
		}
		
		else if (! node->right)
		{
			auto left = node->left;
			
			delete node;
			
			--_size;
			
			return left;
		}
		
		Node* previous = nullptr;
		
		Node* successor = node->right;
		
		while (successor->left)
		{
			previous = successor;
			
			successor = successor->left;
		}
		
		
		if (previous) previous->left = successor->right;
		
		else node->right = successor->right;
		
		
		successor->left = node->left;
		successor->right = node->left;
		
		delete node;
		
		--_size;
		
		return successor;
	}
	
	Node* _copy(Node* other_node)
	{
		if (! other_node) return nullptr;
		
		auto node = new Node(other_node->key,
							 other_node->value);
		
		node->left = _copy(other_node->left);
		
		node->right = _copy(other_node->right);
		
		return node;
	}
	
	size_t _size;
	
	Node* _root;
};

#endif /* RED_BLACK_TREE_HPP */