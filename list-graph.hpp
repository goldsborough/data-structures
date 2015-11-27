#ifndef LIST_GRAPH_HPP
#define LIST_GRAPH_HPP

#include <cstddef>
#include <queue>
#include <vector>

class ListGraph
{
public:
	
	using size_t = std::size_t;
	
	using vertex_t = std::size_t;
	
	struct Edge
	{
		Edge(vertex_t vertex_,
			 size_t edge_)
		: vertex(vertex_)
		, edge(edge_)
		{ }
		
		vertex_t vertex;
		
		size_t edge;
	};
	
	using adjacent_t = std::vector<Edge>;
	
	using iterator = std::vector<adjacent_t>::const_iterator;
	
	
	ListGraph(vertex_t vertices);
	
	ListGraph(vertex_t vertices,
		  std::initializer_list<std::pair<vertex_t, vertex_t>> list);
	
	
	iterator begin() const
	{
		return _vertices.begin();
	}
	
	iterator end() const
	{
		return _vertices.end();
	}

	void add_vertex()
	{
		_vertices.push_back({});
	}
	
	void connect(vertex_t from, vertex_t to)
	{
		_assert_in_range(from);
		_assert_in_range(to);
		
		_vertices[from].push_back({to, _edges});
		
		if (from != to) _vertices[to].push_back({from, _edges});
		
		++_edges;
	}
	
	void disconnect(vertex_t from, vertex_t to)
	{
		_assert_in_range(from);
		_assert_in_range(to);
		
		auto i = std::find_if(_vertices[from].begin(),
							  _vertices[from].end(),
							  [&to] (const Edge& edge) {
								  return edge.vertex == to;
							  });
		
		if (i == _vertices[from].end())
		{
			throw std::invalid_argument("Edges are not connected!");
		}
		
		_vertices[from].erase(i);
		
		if (from != to)
		{
			_vertices[to].erase(std::find_if(_vertices[to].begin(),
											 _vertices[to].end(),
										  	 [&from] (const Edge& edge) {
											  	return edge.vertex == from;
										  	}));
		}
		
		--_edges;
	}
	
	
	const adjacent_t& adjacent(vertex_t vertex) const
	{
		_assert_in_range(vertex);
		
		return _vertices[vertex];
	}
	
	const adjacent_t& operator[](vertex_t vertex) const
	{
		return adjacent(vertex);
	}
	
	
	size_t vertex_number() const
	{
		return _vertices.size();
	}

	size_t edge_number() const
	{
		return _edges;
	}
	
	bool is_empty() const
	{
		return _vertices.empty();
	}
	
private:
	
	void _assert_in_range(vertex_t vertex) const
	{
		if (vertex >= _vertices.size())
		{
			throw std::out_of_range("Vertex out of range!");
		}
	}
	
	std::vector<adjacent_t> _vertices;

	size_t _edges;
};

class Components
{
public:
	
	using size_t = ListGraph::size_t;
	
	using vertex_t = ListGraph::vertex_t;
	
	using component_t = std::vector<vertex_t>;
	
	using iterator = std::vector<component_t>::const_iterator;
	
	
	Components(const ListGraph& graph)
	: _id(graph.vertex_number())
	{
		std::vector<bool> visited(graph.vertex_number());
		
		for (vertex_t vertex = 0; vertex < graph.vertex_number(); ++vertex)
		{
			if (! visited[vertex])
			{
				component_t component;
				
				_dfs(graph, vertex, component, _id.size(), visited);
			}
		}
	}
	
	
	iterator begin() const
	{
		return _components.begin();
	}
	
	iterator end() const
	{
		return _components.end();
	}
	
	
	bool is_connected(vertex_t from, vertex_t to) const
	{
		_assert_in_range(from);
		_assert_in_range(to);
		
		return _id[from] == _id[to];
	}
	
	size_t id(vertex_t vertex)
	{
		_assert_in_range(vertex);
		
		return _id[vertex];
	}
	
	
	const std::vector<component_t>& components() const
	{
		return _components;
	}
	
	const component_t& component(vertex_t vertex) const
	{
		return _components[_id[vertex]];
	}
	
	
	size_t number() const
	{
		return _components.size();
	}
	
private:
	
	void _assert_in_range(vertex_t vertex) const
	{
		if (vertex >= _id.size())
		{
			throw std::out_of_range("Vertex out of range!");
		}
	}
	
	void _dfs(const ListGraph& graph,
			  vertex_t vertex,
			  component_t& component,
			  size_t id,
			  std::vector<bool>& visited)
	{
		if (visited[vertex]) return;
		
		visited[vertex] = true;
		
		component.push_back(vertex);
		
		_id[vertex] = id;
		
		for (const auto& adjacent : graph[vertex])
		{
			_dfs(graph, adjacent.vertex, component, id, visited);
		}
	}
	
	std::vector<size_t> _id;
	
	std::vector<component_t> _components;
};

class ListGraphOperations
{
public:
	
	using size_t = ListGraph::size_t;
	
	using vertex_t = ListGraph::vertex_t;
	
	using component_t = Components::component_t;
	
	using bitset_t = std::vector<bool>;
	
	
	size_t degree(const ListGraph& graph, vertex_t vertex)
	{
		return graph[vertex].size();
	}
	
	size_t max_degree(const ListGraph& graph)
	{
		size_t max = 0;
		
		for (vertex_t vertex = 0; vertex < graph.vertex_number(); ++vertex)
		{
			max = std::max(degree(graph, vertex), max);
		}
		
		return max;
	}
	
	size_t average_degree(const ListGraph& graph)
	{
		return (graph.vertex_number() * 2) / graph.edge_number();
	}
	
	size_t self_loops(const ListGraph& graph)
	{
		size_t count = 0;
		
		for (vertex_t vertex = 0; vertex < graph.vertex_number(); ++vertex)
		{
			count += std::count_if(graph[vertex].begin(),
								   graph[vertex].end(),
								   [&] (const ListGraph::Edge& edge) {
									   return edge.vertex == vertex;
								   });
		}
		
		return count;
	}
	
	bool is_connected(const ListGraph& graph, vertex_t vertex, vertex_t target)
	{
		bitset_t visited(graph.vertex_number());
		
		return _is_connected(graph, vertex, target, visited);
	}
	
	size_t shortest_distance(const ListGraph& graph,
							 vertex_t vertex,
							 vertex_t target)
	{
		if (vertex == target) return 0;
		
		bitset_t visited(graph.vertex_number());
		
		std::queue<vertex_t> queue;
		
		queue.push(vertex);
		
		size_t distance = 0;
		
		vertex_t last_of_level = vertex;
		
		while (! queue.empty())
		{
			vertex = queue.front();
			
			queue.pop();
			
			if (vertex == target) return distance;
			
			visited[vertex] = true;
			
			for (const auto& adjacent : graph[vertex])
			{
				if (! visited[adjacent.vertex])
				{
					queue.push(adjacent.vertex);
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
	
	component_t shortest_path(const ListGraph& graph,
							 vertex_t vertex,
							 vertex_t target)
	{
		if (vertex == target) return {};
		
		bitset_t visited(graph.vertex_number());
		
		std::queue<vertex_t> queue;
		
		queue.push(vertex);
		
		std::vector<vertex_t> source;
		
		source[vertex] = vertex;
		
		size_t distance = 0;
		
		size_t last_of_level = vertex;
		
		while (! queue.empty())
		{
			vertex = queue.front();
			
			queue.pop();
			
			if (vertex == target) break;
			
			visited[vertex] = true;
			
			for (const auto& adjacent : graph[vertex])
			{
				if (! visited[adjacent.vertex])
				{
					source[adjacent.vertex] = vertex;
					
					queue.push(adjacent.vertex);
				}
			}
			
			if (vertex == last_of_level)
			{
				++distance;
				
				last_of_level = queue.back();
			}
		}
		
		component_t path(distance + 1);
		
		for ( ; vertex != source[vertex]; --distance)
		{
			path[distance] = vertex;
			
			vertex = source[vertex];
		}
		
		path[0] = vertex;
		
		return path;
	}
	
	bool euler_tour_possible(const ListGraph& graph)
	{
		if (graph.is_empty()) return false;
		
		Components components(graph);
		
		if (components.number() > 1) return false;
		
		for (vertex_t vertex = 0; vertex < graph.vertex_number(); ++vertex)
		{
			if (degree(graph, vertex) % 2 != 0) return false;
		}
		
		return true;
	}
	
	component_t euler_tour(const ListGraph& graph)
	{
		bitset_t visited(graph.edge_number());
		
		component_t path(graph.edge_number() + 1);
		
		_euler_tour(graph, 0, 0, path, visited);
		
		return path;
	}
	
	bool is_bipartite(const ListGraph& graph,
					  std::function<bool(vertex_t)> predicate)
	{
		bitset_t visited(graph.edge_number());
		
		for (const auto& component : Components(graph))
		{
			auto vertex = component.front();
			
			bool is = predicate(vertex);
			
			if (! _is_bipartite(graph, vertex, is, visited, predicate))
			{
				return false;
			}
		}
		
		return true;
	}

private:
	
	bool _is_bipartite(const ListGraph& graph,
					   vertex_t vertex,
					   bool was,
					   bitset_t& visited,
					   std::function<bool(vertex_t)> predicate)
	{
		bool is = predicate(vertex);
		
		if (is == was) return false;
		
		for (const auto& adjacent : graph[vertex])
		{
			if (! visited[adjacent.edge])
			{
				visited[adjacent.edge] = true;
				
				if (! _is_bipartite(graph, adjacent.vertex, is, visited, predicate))
				{
					return false;
				}
			}
		}
		
		return true;
	}
	
	bool _euler_tour(const ListGraph& graph,
					 vertex_t vertex,
					 size_t edge_count,
					 component_t& path,
					 bitset_t& visited)
	{
		if (edge_count == graph.edge_number())
		{
			path[edge_count] = vertex;
			
			return true;
		}
		
		for (const auto& adjacent : graph[vertex])
		{
			if (! visited[adjacent.edge])
			{
				auto copy = visited;
				
				visited[adjacent.edge] = true;
				
				if (_euler_tour(graph,
								adjacent.vertex,
								edge_count + 1,
								path,
								copy))
				{
					path[edge_count] = vertex;
					
					return true;
				}
			}
		}
		
		return false;
	}
	
	bool _is_connected(const ListGraph& graph,
					   vertex_t vertex,
					   vertex_t target,
					   bitset_t& visited)
	{
		if (visited[vertex]) return false;
		
		if (vertex == target) return true;
		
		visited[vertex] = true;
		
		for (const auto& adjacent : graph[vertex])
		{
			if (_is_connected(graph, adjacent.vertex, target, visited))
			{
				return true;
			}
		}
		
		return false;
	}
};

#endif /* LIST_GRAPH_HPP */