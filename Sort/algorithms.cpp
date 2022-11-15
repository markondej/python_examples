#include <iostream>
#include <cstring>
#include <vector>

template <typename _T>
_T *MergeSort(const _T *data, std::size_t size) {
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
                if ((off_a < size_a) && ((off_b >= size_b) || (group_a[off_a] < group_b[off_b]))) {
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

template <typename _T>
std::size_t Find(const _T &value, const _T *data, std::size_t size) {
    std::size_t begin = 0, end = size;
    while (begin != end) {
        std::size_t offset = begin + (end - begin) / 2;
        if (data[offset] < value) {
            begin = offset + 1;
        } else {
            end = offset;
        }
    }
    if ((begin == size) || (data[begin] != value)) {
        return -1;
    }
    return begin;
};

int main()
{
    std::vector<unsigned> data;
    data.push_back(5);
    data.push_back(3);
    data.push_back(1);
    data.push_back(7);
    data.push_back(2);

    bool first = true;
    auto sorted = MergeSort(data.data(), data.size());
    for (std::size_t i = 0; i < data.size(); i++) {
        std::cout << (first ? "[" : ", ") << sorted[i];
        first = false;
    }
    auto offset = Find(static_cast<unsigned>(5), sorted, data.size());
    std::cout << "]" << std::endl << "5: " << (offset != std::size_t(-1) ? std::to_string(offset + 1) : "Unknown" ) << std::endl;
    delete[] sorted;

    return EXIT_SUCCESS;
}
