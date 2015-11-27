#ifndef MATRIX_GRAPH_HPP
#define MATRIX_GRAPH_HPP

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <stdexcept>

template<std::size_t V>
class MatrixGraph
{
public:
	
	using size_t = std::size_t;
	
	using vertex_t = std::size_t;
	
	using adjacent_t = std::bitset<V>;
	
	
	MatrixGraph()
	{
		std::fill(_matrix.begin(), _matrix.end(), 0);
	}
	
	const adjacent_t& operator[](vertex_t vertex) const
	{
		return adjacent(vertex);
	}
	
	const adjacent_t& adjacent(vertex_t vertex) const
	{
		_assert_in_range(vertex);
		
		return _matrix[vertex];
	}

	void connect(vertex_t from, vertex_t to)
	{
		_assert_in_range(from);
		_assert_in_range(to);
		
		_matrix[from][to] = true;
		
		if (from != to) _matrix[to][from] = true;
		
		++_edges;
	}
	
	void disconnect(vertex_t from, vertex_t to)
	{
		_assert_in_range(from);
		_assert_in_range(to);
		
		if (! connected(from, to))
		{
			throw std::invalid_argument("Vertices are not connected!");
		}
		
		_matrix[from][to] = false;
		_matrix[to][from] = false;
		
		--_edges;
	}
	
	void set(vertex_t from, vertex_t to, bool state)
	{
		if (state) connect(from, to);
		
		else disconnect(from, to);
	}
	
	bool connected(vertex_t from, vertex_t to) const
	{
		_assert_in_range(from);
		_assert_in_range(to);
		
		return _matrix[from][to];
	}
	
	size_t vertex_number() const
	{
		return V;
	}
	
	size_t edge_number() const
	{
		return _edges;
	}
	
private:
	
	void _assert_in_range(vertex_t vertex) const
	{
		if (vertex >= V)
		{
			throw std::invalid_argument("Vertex out of range!");
		}
	}
	
	
	std::array<adjacent_t, V> _matrix;
	
	size_t _edges;
};

template<std::size_t V>
class MatrixGraphOperations
{
public:
	
	using size_t = typename MatrixGraph<V>::size_t;
	
	using vertex_t = typename MatrixGraph<V>::vertex_t;
	
	using component_t = std::vector<vertex_t>;
	
	static size_t degree(const MatrixGraph<V>& graph, vertex_t vertex)
	{
		return std::count(graph[vertex].begin(),
						  graph[vertex].end(),
						  true);
	}
	
	static size_t max_degree(const MatrixGraph<V>& graph)
	{
		size_t max = 0;
		
		for (vertex_t vertex = 0; vertex < graph.vertex_number(); ++vertex)
		{
			max = std::max(max, degree(graph, vertex));
		}
		
		return max;
	}
	
	static inline size_t average_degree(const MatrixGraph<V>& graph)
	{
		return (2.0 * graph.edge_number()) / (graph.vertex_number());
	}
	
	static size_t self_loops(const MatrixGraph<V>& graph)
	{
		size_t loops = 0;
		
		for (vertex_t vertex = 0; vertex < graph.vertex_number(); ++vertex)
		{
			if (graph.connected(vertex, vertex)) ++loops;
		}
		
		return loops;
	}
	
	static bool connected(const MatrixGraph<V>& graph,
						  vertex_t vertex,
						  vertex_t target)
	{
		if (vertex == target) return true;
		
		std::vector<bool> visited(graph.vertex_number());
		
		return _connected(graph, vertex, target, visited);
	}
	
	static size_t shortest_distance(const MatrixGraph<V>& graph,
									vertex_t vertex,
									vertex_t target)
	{
		if (vertex == target) return 0;
		
		std::queue<vertex_t> queue;
		
		queue.push(vertex);
		
		std::bitset<V> visited;
		
		size_t distance = 1;
		
		vertex_t last_of_level = vertex;
		
		while (! queue.empty())
		{
			vertex = queue.front();
			
			queue.pop();
			
			if (graph.connected(vertex, target)) return distance;
			
			visited[vertex] = true;
			
			for (vertex_t other = 0; other < graph.vertex_number(); ++other)
			{
				if (graph.connected(vertex, other) && ! visited[other])
				{
					queue.push(other);
				}
			}
			
			if (vertex == last_of_level)
			{
				++distance;
				
				last_of_level = queue.back();
			}
		}
		
		throw std::invalid_argument("Vertices are not connected!");
	}
	
	static component_t shortest_path(const MatrixGraph<V>& graph,
									 vertex_t vertex,
									 vertex_t target)
	{
		if (vertex == target) return {};
		
		std::queue<vertex_t> queue;
		
		queue.push(vertex);
		
		std::bitset<V> visited;
		
		component_t source(graph.vertex_number());
		
		size_t distance = 1;
		
		vertex_t last_of_level = vertex;
		
		while (! queue.empty())
		{
			vertex = queue.front();
			
			queue.pop();
			
			if (graph.connected(vertex, target)) break;
			
			visited[vertex] = true;
			
			for (vertex_t other = 0; other < graph.vertex_number(); ++other)
			{
				if (graph.connected(vertex, other) && ! visited[other])
				{
					source[other] = vertex;
					
					queue.push(other);
				}
			}
			
			if (vertex == last_of_level)
			{
				++distance;
				
				last_of_level = queue.back();
			}
		}
		
		component_t path(distance + 1);
		
		path[distance] = target;
		
		for (long v = distance - 1; v >= 0; --v)
		{
			path[v] = vertex;
			
			vertex = source[vertex];
		}
		
		return path;
	}
	
private:
	
	static bool _connected(const MatrixGraph<V>& graph,
						  vertex_t vertex,
						  vertex_t target,
						  std::vector<bool>& visited)
	{
		if (visited[vertex]) return false;
		
		if (graph.connected(vertex, target)) return true;
		
		visited[vertex] = true;
		
		for (vertex_t other = 0; other < graph.vertex_number(); ++other)
		{
			if (graph.connected(vertex, other))
			{
				return _connected(graph, other, target, visited);
			}
		}
		
		return false;
	}

};

#endif /* MATRIX_GRAPH_HPP */