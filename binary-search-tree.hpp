#ifndef BINARY_SEARCH_TREE_HPP
#define BINARY_SEARCH_TREE_HPP

#include <initializer_list>
#include <stdexcept>

template<typename Key, typename Value>
class BinarySearchTree
{
public:
	
	using size_t = std::size_t;
	
	BinarySearchTree()
	: _size(0)
	, _root(nullptr)
	{ }
	
	BinarySearchTree(std::initializer_list<std::pair<Key, Value>> list)
	: BinarySearchTree()
	{
		for (const auto& item : list) insert(item.first, item.second);
	}
	
	BinarySearchTree(const BinarySearchTree& other)
	: _size(other._size)
	, _root(_copy(other._root))
	{ }
	
	BinarySearchTree(BinarySearchTree&& other) noexcept
	: BinarySearchTree()
	{
		swap(other);
	}
	
	BinarySearchTree& operator=(BinarySearchTree other)
	{
		swap(other);
		
		return *this;
	}
	
	void swap(BinarySearchTree& other) noexcept
	{
		// Enable ADL
		using std::swap;
		
		
		swap(_root, other._root);
		
		swap(_size, other._size);
	}
	
	friend void swap(BinarySearchTree& first, BinarySearchTree& second) noexcept
	{
		first.swap(second);
	}
	
	~BinarySearchTree()
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
		Node(const Key& key_,
			 const Value& value_ = Value(),
			 Node* left_ = nullptr,
			 Node* right_ = nullptr)
		: key(key_)
		, value(value_)
		, left(left_)
		, right(right_)
		{ }
		
		
		Key key;
		
		Value value;
		
		
		Node* left;
		
		Node* right;
	};
	
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
		
		return node;
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
			delete node;
			
			node = new_node;
		}
		
		return node;
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
		
		else return get_successor(node);
		
		return node;
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

template<typename Node>
Node successor(Node node)
{
	if (! node) return node;
	
	if (node->right)
	{
		node = node->right;
		
		while (node->left) node = node->left;
	}
	
	else
	{
		while (node->parent && node != node->parent->left)
		{
			node = node->parent;
		}
		
		node = node->parent;
	}
	
	return node;
}


#endif /* BINARY_SEARCH_TREE_HPP */