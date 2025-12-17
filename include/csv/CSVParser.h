#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <tuple>
#include <array>
#include <utility>

class CSVParser {
private:
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> data;
    std::unordered_map<std::string, size_t> column_indices;

public:
    CSVParser() = default;

    static CSVParser read_csv(const std::string& filename,
                              char delimiter = ',',
                              bool has_header = true) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::vector<std::string> columns;
        std::vector<std::vector<std::string>> data;
        size_t expected_columns = 0;

        if (has_header) {
            columns = tokenise_csv(file, delimiter);
            expected_columns = columns.size();

            if (expected_columns == 0) {
                throw std::runtime_error("CSV header is empty");
            }
        }

        while (file) {
            auto row = tokenise_csv(file, delimiter);
            if (!file && row.size() == 1 && row[0].empty()) 
                break;

            if (row.size() == 1 && row[0].empty())
                continue;

            if (!has_header && expected_columns == 0)
                expected_columns = row.size();

            if (row.size() != expected_columns) {
                throw std::runtime_error(
                    "Malformed CSV row: expected " +
                    std::to_string(expected_columns) +
                    " columns, got " +
                    std::to_string(row.size())
                );
            }

            data.push_back(std::move(row));
        }

        return CSVParser(columns, data);
    }

    CSVParser(const std::vector<std::string>& cols,
              const std::vector<std::vector<std::string>>& rows)
        : columns(cols), data(rows) {
        for (size_t i = 0; i < columns.size(); ++i) {
            column_indices.emplace(columns[i], i);
        }
    }

    // CSV tokenizer (state machine)
    static std::vector<std::string> tokenise_csv(
        std::istream& input,
        char delimiter = ','
    ) {
        std::vector<std::string> row;
        std::string field;
        char c;

        enum class State {
            StartField,
            InField,
            InQuotedField,
            QuoteInQuoted
        };

        State state = State::StartField;

        while (input.get(c)) {
            switch (state) {

            case State::StartField:
                if (c == delimiter) {
                    row.emplace_back("");
                } else if (c == '"') {
                    state = State::InQuotedField;
                } else if (c == '\n') {
                    row.emplace_back("");
                    return row;
                } else if (c == '\r') {
                    continue;
                } else {
                    field += c;
                    state = State::InField;
                }
                break;

            case State::InField:
                if (c == delimiter) {
                    row.push_back(std::move(field));
                    field.clear();
                    state = State::StartField;
                } else if (c == '\n') {
                    row.push_back(std::move(field));
                    return row;
                } else if (c == '\r') {
                    continue;
                } else {
                    field += c;
                }
                break;

            case State::InQuotedField:
                if (c == '"') {
                    state = State::QuoteInQuoted;
                } else {
                    field += c;
                }
                break;

            case State::QuoteInQuoted:
                if (c == '"') {
                    field += '"';
                    state = State::InQuotedField;
                } else if (c == delimiter) {
                    row.push_back(std::move(field));
                    field.clear();
                    state = State::StartField;
                } else if (c == '\n') {
                    row.push_back(std::move(field));
                    return row;
                } else if (c == '\r') {
                    continue;
                } else {
                    throw std::runtime_error("Malformed CSV: unexpected character after quote");
                }
                break;
            }
        }

        if (state == State::InQuotedField) {
            throw std::runtime_error("Malformed CSV: unterminated quoted field");
        }

        row.push_back(std::move(field));
        return row;
    }

    const std::vector<std::string>& get_columns() const noexcept { return columns; }
    const std::vector<std::vector<std::string>>& rows() const noexcept { return data; }

    size_t row_count() const noexcept { return data.size(); }
    size_t column_count() const noexcept { return columns.size(); }

    std::vector<std::string> get_column(const std::string& col_name) const {
        auto it = column_indices.find(col_name);
        if (it == column_indices.end()) {
            throw std::runtime_error("Column not found: " + col_name);
        }

        std::vector<std::string> result;
        result.reserve(data.size());
        for (const auto& row : data) {
            result.push_back(row[it->second]);
        }
        return result;
    }

    std::vector<std::string> get_row(size_t index) const {
        if (index >= data.size()) {
            throw std::runtime_error("Row index out of range");
        }
        return data[index];
    }

    /*
        * Row-wise transformation and selection
        *
        * These methods provides higher-order operations over the CSV rows.
        *
        * - filter(...) produces a new CSVParser instance containing only
        *   the rows for which the user-supplied predicate returns true.
        *
        * - apply(...) performs an in-place transformation on a single column
        *   by invoking a user-supplied function on each cell in that column.
        *
        * These operations are row-oriented and do not alter the column schema.
        * Filtering preserves column order and names, while transformation
        * affects only the targeted column values.
        *
        * All bounds checking and column lookup validation is performed prior
        * to execution; invalid column names result in runtime errors.
    */

    CSVParser filter(const std::function<bool(const std::vector<std::string>&)>& condition) const {
        std::vector<std::vector<std::string>> filtered;
        for (const auto& row : data) {
            if (condition(row)) {
                filtered.push_back(row);
            }
        }
        return CSVParser(columns, filtered);
    }

    void apply(const std::string& col_name,
               const std::function<void(std::string&)>& func) {
        auto it = column_indices.find(col_name);
        if (it == column_indices.end()) {
            throw std::runtime_error("Column not found: " + col_name);
        }

        for (auto& row : data) {
            func(row[it->second]);
        }
    }

    // Statistics
    double sum(const std::string& col_name) const {
        auto values = to_double_vector(get_column(col_name));
        return std::accumulate(values.begin(), values.end(), 0.0);
    }

    double mean(const std::string& col_name) const {
        auto values = to_double_vector(get_column(col_name));
        if (values.empty()) {
            throw std::runtime_error("Cannot compute mean of empty column");
        }
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    double min(const std::string& col_name) const {
        auto values = to_double_vector(get_column(col_name));
        if (values.empty()) 
            throw std::runtime_error("Cannot compute min of empty column");
        
        return *std::min_element(values.begin(), values.end());
    }

    double max(const std::string& col_name) const {
        auto values = to_double_vector(get_column(col_name));
        if (values.empty()) 
            throw std::runtime_error("Cannot compute min of empty column");
        return *std::max_element(values.begin(), values.end());
    }

    void print(size_t num_rows = 5) const {
        for (const auto& col : columns) {
            std::cout << std::setw(15) << col << " ";
        }
        std::cout << '\n';

        for (size_t i = 0; i < std::min(num_rows, data.size()); ++i) {
            for (const auto& cell : data[i]) {
                std::cout << std::setw(15) << cell << " ";
            }
            std::cout << '\n';
        }
    }

    /*
        * Object construction from CSV rows
        *
        * This section provides utilities to construct user-defined objects
        * directly from CSV data.
        *
        * Each row of the CSV is interpreted as a sequence of constructor
        * arguments. The caller specifies the target type T and the expected
        * argument types Args..., and each field in the row is converted to
        * the corresponding type using convert<T>().
        *
        * The mapping is positional:
        *   - Column 0 → constructor argument 0
        *   - Column 1 → constructor argument 1
        *   - ...
        *
        * No column-name matching is performed.
        *
        * Constraints and behavior:
        *   - The number of CSV columns must exactly match the number of
        *     constructor arguments (sizeof...(Args)).
        *   - Conversion failures (e.g. invalid numeric values) throw.
        *   - Rows with mismatched arity throw at runtime.
        *
        * This mechanism is intentionally low-level and order-dependent.
        * It is suitable for tightly controlled CSV formats or internal tools,
        * but not for loosely structured or evolving schemas.
    */

    template <typename T, typename... Args>
    std::vector<T> create_objects() const {
        std::vector<T> objects;
        objects.reserve(data.size());

        for (const auto& row : data) {
            if (row.size() != sizeof...(Args)) {
                throw std::runtime_error("Mismatched constructor arity");
            }
            objects.emplace_back(
                from_row<T, Args...>(row, std::index_sequence_for<Args...>{})
            );
        }
        return objects;
    }

    template <typename T, typename... Args>
    static T create_object(const std::vector<std::string>& tokens) {
        if (tokens.size() != sizeof...(Args)) {
            throw std::runtime_error("Mismatched constructor arity");
        }
        return from_row<T, Args...>(tokens, std::index_sequence_for<Args...>{});
    }

    template <typename T>
    static T convert(const std::string&) = delete;

private:
    std::vector<double> to_double_vector(const std::vector<std::string>& vec) const {
        std::vector<double> result;
        result.reserve(vec.size());
        for (const auto& s : vec) {
            result.push_back(std::stod(s));
        }
        return result;
    }

    template <typename T, typename... Args, std::size_t... I>
    static T from_row(const std::vector<std::string>& tokens,
                      std::index_sequence<I...>) {
        return T{ convert<typename std::tuple_element<I, std::tuple<Args...>>::type>(tokens[I])... };
    }
};

template <>
inline int CSVParser::convert<int>(const std::string& s) { return std::stoi(s); }

template <>
inline long CSVParser::convert<long>(const std::string& s) { return std::stol(s); }

template <>
inline long long CSVParser::convert<long long>(const std::string& s) { return std::stoll(s); }

template <>
inline float CSVParser::convert<float>(const std::string& s) { return std::stof(s); }

template <>
inline double CSVParser::convert<double>(const std::string& s) { return std::stod(s); }

template <>
inline std::string CSVParser::convert<std::string>(const std::string& s) { return s; }

#endif // CSVPARSER_H
