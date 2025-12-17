#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <tuple>

class CSVParser {
private:
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> data;
    std::unordered_map<std::string, size_t> column_indices;

public:
    CSVParser() {}

    // Static method to read and parse a CSV file into a DataFrame-like structure
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

        while (file.peek() != EOF) {
            auto row = tokenise_csv(file, delimiter);

            if (row.size() == 1 && row[0].empty()) {
                continue;
            }

            if (!has_header && expected_columns == 0) {
                expected_columns = row.size();
            }

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


    // Constructor initializing columns and data
    CSVParser(const std::vector<std::string>& cols, const std::vector<std::vector<std::string>>& data) 
        : columns(cols), data(data) {
        for (size_t i = 0; i < columns.size(); ++i) {
            column_indices[columns[i]] = i;
        }
    }

    // Generic tokenizing method
    // Returns a vector of strings that should be casted later if necessary
    static std::vector<std::string> tokenise(
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

        // End-of-file handling
        if (state == State::InQuotedField) {
            throw std::runtime_error("Malformed CSV: unterminated quoted field");
        }

        row.push_back(std::move(field));
        return row;
    }

    // Retrieve a column as a vector of strings
    std::vector<std::string> get_column(const std::string& col_name) const {
        if (column_indices.find(col_name) == column_indices.end()) {
            throw std::runtime_error("Column not found: " + col_name);
        }

        std::vector<std::string> column_data;
        size_t index = column_indices.at(col_name);

        for (const auto& row : data) {
            column_data.push_back(row[index]);
        }

        return column_data;
    }

    // Retrieve a row by index
    std::vector<std::string> get_row(size_t index) const {
        if (index >= data.size()) {
            throw std::runtime_error("Row index out of range");
        }
        return data[index];
    }

    // Filter rows based on a condition
    CSVParser filter(const std::function<bool(const std::vector<std::string>&)>& condition) const {
        std::vector<std::vector<std::string>> filtered_data;
        for (const auto& row : data) {
            if (condition(row)) {
                filtered_data.push_back(row);
            }
        }
        return CSVParser(columns, filtered_data);
    }

    const std::vector<std::string>& get_columns() const noexcept {
        return columns;
    }

    const std::vector<std::vector<std::string>>& rows() const noexcept {
        return data;
    }

    size_t row_count() const noexcept {
        return data.size();
    }

    size_t column_count() const noexcept {
        return columns.size();
    }

    // Apply a function to transform a column
    void apply(const std::string& col_name,
           const std::function<void(std::string&)>& func) {
        auto it = column_indices.find(col_name);
        if (it == column_indices.end()) {
            throw std::runtime_error("Column not found: " + col_name);
        }

        size_t index = it->second;
        for (auto& row : data) {
            func(row[index]);
        }
    }

    // Basic descriptive statistics:
    // Calculate the mean of a numeric column
    double mean(const std::string& col_name) const {
        std::vector<std::string> col = get_column(col_name);
        std::vector<double> values = to_double_vector(col);
        double sum = std::accumulate(values.begin(), values.end(), 0.0);

        if (values.empty())
            throw std::runtime_error("Cannot compute mean of empty column");
    }

    // Calculate the sum of a numeric column
    double sum(const std::string& col_name) const {
        std::vector<std::string> col = get_column(col_name);
        std::vector<double> values = to_double_vector(col);
        return std::accumulate(values.begin(), values.end(), 0.0);
    }

    // Find the minimum value in a numeric column
    double min(const std::string& col_name) const {
        std::vector<std::string> col = get_column(col_name);
        std::vector<double> values = to_double_vector(col);
        return *std::min_element(values.begin(), values.end());
    }

    // Find the maximum value in a numeric column
    double max(const std::string& col_name) const {
        std::vector<std::string> col = get_column(col_name);
        std::vector<double> values = to_double_vector(col);
        return *std::max_element(values.begin(), values.end());
    }

    // Print the DataFrame-like structure
    void print(size_t num_rows = 5) const {
        for (const auto& col : columns) {
            std::cout << std::setw(15) << col << " ";
        }
        std::cout << std::endl;

        for (size_t i = 0; i < std::min(num_rows, data.size()); ++i) {
            for (const auto& item : data[i]) {
                std::cout << std::setw(15) << item << " ";
            }
            std::cout << std::endl;
        }
    }

    // Method to create an object from CSV row data using parameter packing
    template <typename T, typename... Args>
    std::vector<T> create_objects() const {
        std::vector<T> objects;
        for (const auto& row : data) {
            if (row.size() != sizeof...(Args)) {
                throw std::runtime_error("Mismatched number of tokens for the expected constructor arguments.");
            }
            objects.emplace_back(from_row<T, Args...>(row, std::index_sequence_for<Args...>{}));
        }
        return objects;
    }

    // Creates an object of type T by unpacking the tokens into the constructor
    template <typename T, typename... Args>
    static T create_object(const std::vector<std::string>& tokens) {
        if (tokens.size() != sizeof...(Args)) {
            throw std::runtime_error("Mismatched number of tokens for the expected constructor arguments.");
        }
        return from_row<T, Args...>(tokens, std::index_sequence_for<Args...>{});
    }

    // Converts a string token to the desired type
    template <typename T>
    static T convert(const std::string&) = delete;

    template <>
    inline int convert<int>(const std::string& s) {
        return std::stoi(s);
    }

    template <>
    inline long convert<long>(const std::string& s) {
        return std::stol(s);
    }

    template <>
    inline long long convert<long long>(const std::string& s) {
        return std::stoll(s);
    }

    template <>
    inline double convert<double>(const std::string& s) {
        return std::stod(s);
    }

    template <>
    inline float convert<float>(const std::string& s) {
        return std::stof(s);
    }

    template <>
    inline std::string convert<std::string>(const std::string& s) {
        return s;
    }


private:
    // Casting to convert a vector of strings to a vector of doubles
    std::vector<double> to_double_vector(const std::vector<std::string>& vec) const {
        std::vector<double> result;
        std::transform(vec.begin(), vec.end(), std::back_inserter(result), [](const std::string& val) {
            return std::stod(val);
        });
        return result;
    }

    // Helper function to unpack the tokens and create the object
    template <typename T, typename... Args, std::size_t... I>
    static T from_row(const std::vector<std::string>& tokens, std::index_sequence<I...>) {
        return T{convert<typename std::tuple_element<I, std::tuple<Args...>>::type>(tokens[I])...};
    }
};

#endif // CSVPARSER_H
