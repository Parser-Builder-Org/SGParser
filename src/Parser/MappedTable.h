// Filename:  MappedTable.h
// Content:   General purpose mapped table
// Provided AS IS under MIT License; see LICENSE file in root folder.

#ifndef INC_SGPARSER_MAPPEDTABLE_H
#define INC_SGPARSER_MAPPEDTABLE_H

#include <unordered_map>

namespace SGParser
{

// *** MappedTable class

template <class T, T EmptyValue = T{}>
class MappedTable final {
public:
    using value_type = T;

private:
    using map_type       = std::unordered_map<size_t, value_type>;

public:
    using iterator       = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

public:
    // *** Value access

    // Get the value from the structure (should be fast!)
    const value_type& GetValue(size_t pos) const {
        const auto it = values.find(pos);
        return it != values.end() ? it->second : emptyValue;
    }

    // Value assignment
    void SetValue(size_t pos, const value_type& value) {
        // Insert/replace a value
        if (value != EmptyValue)
            values.insert_or_assign(pos, value);
        else // Remove an existing value
            values.erase(pos);
    }

    // Check if the table has a value for a specific position
    bool HasValue(size_t pos) const {
        return values.find(pos) != values.end();
    }

    // Return the empty value used by this structure
    constexpr value_type GetEmptyValue() const noexcept { return EmptyValue; }

    // *** Iterators

    iterator       begin() noexcept       { return values.begin(); }
    iterator       end() noexcept         { return values.end(); }
    const_iterator begin() const noexcept { return values.cbegin(); }
    const_iterator end() const noexcept   { return values.cend(); }

    // *** Utility functions

    // Return the number of valid values
    size_t size() const noexcept             { return values.size(); }
    // Return whether or not the table is empty
    bool   empty() const noexcept            { return values.empty(); }
    // Clear the table
    void   clear() noexcept                  { values.clear(); }
    // Swap the tables
    void   swap(MappedTable& other) noexcept { values.swap(other.values); }

private:
    const value_type emptyValue = EmptyValue;

    map_type values;
};

} // namespace SGParser

#endif // INC_SGPARSER_MAPPEDTABLE_H
