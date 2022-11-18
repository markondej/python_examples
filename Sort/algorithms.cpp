#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

template <typename _T, class _Fn>
_T *MergeSort(const _T *data, std::size_t size, const _Fn &compare) {
    if (!size) {
        return nullptr;
    }
    _T *sorted = new _T[size];
    std::memcpy(sorted, data, sizeof(_T) * size);
    std::size_t groups = size, group_size = 1;
    while (groups > 1) {
        std::size_t merged_cnt = 0;
        for (std::size_t i = 0; i < groups / 2; i++) {
            std::size_t off_a = 0, off_b = 0, off_m = 0,
                size_a = group_size, size_b = (i * 2 < groups - 2) ? group_size : size - group_size * (groups - 1);
            _T *group_a = &sorted[i * 2 * group_size],
                *group_b = &sorted[(i * 2 + 1) * group_size],
                *merged = new _T[size_a + size_b];
            while ((off_a < size_a) || (off_b < size_b)) {
                if ((off_a < size_a) && ((off_b >= size_b) || compare(group_a[off_a], group_b[off_b]))) {
                    merged[off_m++] = group_a[off_a++];
                } else {
                    merged[off_m++] = group_b[off_b++];
                }
            }
            std::memcpy(group_a, merged, sizeof(_T) * (size_a + size_b));
            delete[] merged;
            merged_cnt++;
        }
        groups -= merged_cnt;
        group_size *= 2;
    }
    return sorted;
}

template <typename _T, class _Fn_c, class _Fn_v>
std::size_t Find(const _T &value, const _T *data, std::size_t size, const _Fn_c &compare, const _Fn_v &verify) {
    std::size_t begin = 0, end = size;
    while (begin != end) {
        std::size_t offset = begin + (end - begin) / 2;
        if (compare(data[offset], value)) {
            begin = offset + 1;
        } else {
            end = offset;
        }
    }
    if ((begin == size) || !verify(data[begin], value)) {
        return SIZE_MAX;
    }
    return begin;
};

struct Connection {
    std::size_t vertexes[2];
    unsigned distance;
};

std::vector<Connection> FindRoute(const std::vector<Connection> &connections, std::size_t start, std::size_t end) {
    if (connections.empty()) {
        return std::vector<Connection>();
    }

    auto sorted_by_src = MergeSort(connections.data(), connections.size(), [](const Connection &a, const Connection &b) -> bool {
        return a.vertexes[0] < b.vertexes[0];
    });
    auto sorted_by_dst = MergeSort(connections.data(), connections.size(), [](const Connection &a, const Connection &b) -> bool {
        return a.vertexes[1] < b.vertexes[1];
    });

    struct Vertex {
        std::size_t vertex, previous;
        unsigned distance;
        bool visited;
    };
    std::vector<Vertex> vertexes;
    std::size_t src_off = 0, dst_off = 0, prev_vertex = SIZE_MAX;
    while ((src_off < connections.size()) || (dst_off < connections.size())) {
        std::size_t src_vertex = (src_off < connections.size()) ? sorted_by_src[src_off].vertexes[0] : SIZE_MAX,
            dst_vertex = (dst_off < connections.size()) ? sorted_by_dst[dst_off].vertexes[1] : SIZE_MAX;
        if (src_vertex < dst_vertex) {
            if ((prev_vertex == SIZE_MAX) || (prev_vertex < src_vertex)) {
                vertexes.push_back({ src_vertex, UINT_MAX, UINT_MAX, false });
                prev_vertex = src_vertex;
            }
            src_off++;
        } else {
            if ((prev_vertex == SIZE_MAX) || (prev_vertex < dst_vertex)) {
                vertexes.push_back({ dst_vertex, SIZE_MAX, UINT_MAX, false });
                prev_vertex = dst_vertex;
            }
            dst_off++;
        }
    }

    auto findVertexOffset = [&](std::size_t vertex) -> std::size_t {
        return Find(Vertex({ vertex }), vertexes.data(), vertexes.size(), [](const Vertex &a, const Vertex &b) -> bool {
            return a.vertex < b.vertex;
        }, [](const Vertex &a, const Vertex &b) -> bool {
            return a.vertex == b.vertex;
        });
    };

    std::size_t offset = findVertexOffset(start);
    if (offset == SIZE_MAX) {
        delete[] sorted_by_src, sorted_by_dst;
        return std::vector<Connection>();
    }
    vertexes[offset].distance = 0;

    auto updateDistances = [&]() {
        std::size_t src_off = Find(Connection({ { vertexes[offset].vertex, SIZE_MAX } }), sorted_by_src, connections.size(), [](const Connection &a, const Connection &b) -> bool {
            return a.vertexes[0] < b.vertexes[0];
        }, [](const Connection &a, const Connection &b) -> bool {
            return a.vertexes[0] == b.vertexes[0];
        });
        if (src_off != SIZE_MAX) {
            while ((sorted_by_src[src_off].vertexes[0] == vertexes[offset].vertex) && (src_off < connections.size())) {
                std::size_t vertex_off = findVertexOffset(sorted_by_src[src_off].vertexes[1]);
                if (vertexes[vertex_off].distance > vertexes[offset].distance + sorted_by_src[src_off].distance) {
                    vertexes[vertex_off].distance = vertexes[offset].distance + sorted_by_src[src_off].distance;
                    vertexes[vertex_off].previous = vertexes[offset].vertex;
                }
                src_off++;
            }
        }
        std::size_t dst_off = Find(Connection({ { SIZE_MAX, vertexes[offset].vertex } }), sorted_by_dst, connections.size(), [](const Connection &a, const Connection &b) -> bool {
            return a.vertexes[1] < b.vertexes[1];
        }, [](const Connection &a, const Connection &b) -> bool {
            return a.vertexes[1] == b.vertexes[1];
        });
        if (dst_off != SIZE_MAX) {
            while ((sorted_by_dst[dst_off].vertexes[1] == vertexes[offset].vertex) && (dst_off < connections.size())) {
                std::size_t vertex_off = findVertexOffset(sorted_by_dst[dst_off].vertexes[0]);
                if (vertexes[vertex_off].distance > vertexes[offset].distance + sorted_by_dst[dst_off].distance) {
                    vertexes[vertex_off].distance = vertexes[offset].distance + sorted_by_dst[dst_off].distance;
                    vertexes[vertex_off].previous = vertexes[offset].vertex;
                }
                dst_off++;
            }
        }
    };
    auto findNextUnvisited = [&]() -> std::size_t {
        std::size_t found = SIZE_MAX;
        unsigned distance = UINT_MAX;
        for (std::size_t offset = 0; offset < vertexes.size(); offset++) {
            if (!vertexes[offset].visited && (vertexes[offset].distance < distance)) {
                distance = vertexes[offset].distance;
                found = offset;
            }
        }
        return found;
    };
    while (true) {
        updateDistances();
        std::size_t previous = vertexes[offset].vertex;
        vertexes[offset].visited = true;
        offset = findNextUnvisited();
        if (offset == SIZE_MAX) {
            break;
        }
    }

    delete[] sorted_by_src, sorted_by_dst;

    offset = findVertexOffset(end);
    if ((offset == SIZE_MAX) || !vertexes[offset].visited) {
        return std::vector<Connection>();
    }

    std::vector<Connection> route;
    std::size_t previous = end;
    unsigned distance = vertexes[offset].distance;
    while (vertexes[offset].previous != SIZE_MAX) {
        offset = findVertexOffset(vertexes[offset].previous);
        route.push_back({ { vertexes[offset].vertex, previous }, distance - vertexes[offset].distance });
        previous = vertexes[offset].vertex;
        distance = vertexes[offset].distance;
    }

    std::reverse(route.begin(), route.end());
    return route;
}

int main()
{
    std::vector<unsigned> data;
    data.push_back(5);
    data.push_back(3);
    data.push_back(1);
    data.push_back(7);
    data.push_back(2);

    std::cout << "ARRAY:" << std::endl;
    bool first = true;
    auto sorted = MergeSort(data.data(), data.size(), std::less<>());
    for (std::size_t i = 0; i < data.size(); i++) {
        std::cout << (first ? "[" : ", ") << sorted[i];
        first = false;
    }
    auto offset = Find(static_cast<unsigned>(3), sorted, data.size(), std::less<>(), std::equal_to<>());
    std::cout << "]" << std::endl << "3: " << (offset != std::size_t(-1) ? std::to_string(offset + 1) : "Unknown" ) << std::endl;
    delete[] sorted;

    std::vector<Connection> graph;
    graph.push_back({ { 0, 1 }, 6 });
    graph.push_back({ { 1, 2 }, 5 });
    graph.push_back({ { 1, 3 }, 2 });
    graph.push_back({ { 0, 3 }, 1 });
    graph.push_back({ { 3, 4 }, 1 });
    graph.push_back({ { 4, 1 }, 2 });
    graph.push_back({ { 4, 2 }, 5 });

    std::cout << "GRAPH:" << std::endl;
    for (auto &connection : graph) {
        std::cout << connection.vertexes[0] << "->" << connection.vertexes[1] << ": " << connection.distance << std::endl;
    }
    first = true;
    auto route = FindRoute(graph, 0, 2);
    std::cout << "0->2: ";
    for (std::size_t i = 0; i < route.size(); i++) {
        std::cout << (first ? "[" : ", ") << route[i].vertexes[0] << "->" << route[i].vertexes[1];
        first = false;
    }
    std::cout << "]" << std::endl;

    std::vector<int> vec;
    std::sort(vec.begin(), vec.end(), [](const int &a, const int &b) -> bool {
        return a < b;
    });


    return EXIT_SUCCESS;
}
