#include "../CSVParser.h"
#include "class_ex.h" 

/*
====================================================================================================
THIS IS A EXAMPLE OF USAGE OF THIS LIBRARY, IT WAS TESTED WITH A DATASET OBTAINED 
FROM KAGGLE THAT YOU CAN CHECKOUT HERE:
https://www.kaggle.com/datasets/myrios/cost-of-living-index-by-country-by-number-2024
====================================================================================================
*/

int main() {
    // Loading the CSV
    CSVParser df = CSVParser::read_csv("CLIdx.csv");

    // Print the first few rows of the DataFrame
    df.print();

    // Retrieve a specific column
    std::vector<std::string> names = df.get_column("Country");

    // Apply a function to modify a column (e.g., converting to uppercase)
    df.apply("Country", [](std::string& s) { std::transform(s.begin(), s.end(), s.begin(), ::toupper); });

    // Print the modified DataFrame
    df.print();

    // Filter rows where rank is less than 10
    CSVParser filtered_df = df.filter([](const std::vector<std::string>& row) {
        return std::stoi(row[0]) < 10;
    });

    // Print the filtered DataFrame
    filtered_df.print();

    // Basic statistics
    double average_cost = df.mean("Rent Index");
    double total_cost = df.sum("Rent Index");
    double min_cost = df.min("Rent Index");
    double max_cost = df.max("Rent Index");

    std::cout << "Mean cost: " << average_cost << std::endl;
    std::cout << "Total cost: " << total_cost << std::endl;
    std::cout << "Min cost: " << min_cost << std::endl;
    std::cout << "Max cost: " << max_cost << std::endl;

    std::vector<Country_Indices> objects = df.create_objects<Country_Indices, int, std::string, double, double, double, double, double, double>();

    return 0;
}
