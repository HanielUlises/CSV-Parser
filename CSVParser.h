#ifndef CSVPARSER_H
#define CSVPARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>
#include <functional>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include <tuple>

class CSVParser {
private:
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> data;
    std::map<std::string, size_t> column_indices;

public:
    CSVParser() {}

    // Static method to read and parse a CSV file into a DataFrame-like structure
    static CSVParser read_csv(const std::string& filename, char delimiter = ',', bool hasHeader = true) {
        std::ifstream file{filename};
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the file " + filename);
        }

        std::string line;
        std::vector<std::string> columns;
        std::vector<std::vector<std::string>> data;

        // Retrieving the header of the CSV data
        if (hasHeader && std::getline(file, line)) {
            columns = tokenise(line, delimiter);
        }
        // Populating the matrix of data from the CSV
        while (std::getline(file, line)) {
            data.push_back(tokenise(line, delimiter));
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
    static std::vector<std::string> tokenise(const std::string& line, char delimiter) {
        std::vector<std::string> tokens;
        std::istringstream stream(line);
        std::string token;

        while (std::getline(stream, token, delimiter)) {
            tokens.push_back(token);
        }

        return tokens;
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

    // Apply a function to transform a column
    void apply(const std::string& col_name, const std::function<void(std::string&)>& func) {
        size_t index = column_indices.at(col_name);
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
        return sum / values.size();
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
            objects.emplace_back(createObjectHelper<T, Args...>(row, std::index_sequence_for<Args...>{}));
        }
        return objects;
    }

    // Creates an object of type T by unpacking the tokens into the constructor
    template <typename T, typename... Args>
    static T createObject(const std::vector<std::string>& tokens) {
        if (tokens.size() != sizeof...(Args)) {
            throw std::runtime_error("Mismatched number of tokens for the expected constructor arguments.");
        }
        return createObjectHelper<T, Args...>(tokens, std::index_sequence_for<Args...>{});
    }

    // Helper function to unpack the tokens and create the object
    template <typename T, typename... Args, std::size_t... I>
    static T createObjectHelper(const std::vector<std::string>& tokens, std::index_sequence<I...>) {
        return T{convert<typename std::tuple_element<I, std::tuple<Args...>>::type>(tokens[I])...};
    }

    // Converts a string token to the desired type
    template <typename T>
    static T convert(const std::string& token) {
        if constexpr (std::is_integral<T>::value) {
            return std::stoi(token);
        } else if constexpr (std::is_floating_point<T>::value) {
            return std::stod(token);
        } else {
            return token;
        }
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
};

#endif // CSVPARSER_H
