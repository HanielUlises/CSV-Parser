# CSV Parser Project

## Overview

This project is a C++ application that demonstrates how to parse and manipulate CSV files using a custom CSV parsing library. It showcases loading CSV data into a DataFrame-like structure, applying transformations, filtering rows, calculating statistics, and creating custom objects from the CSV data.

## Features

- **Load CSV Data**: Read data from a CSV file into a DataFrame-like structure.
- **Print Data**: Output the DataFrame contents to the console.
- **Column Manipulation**: Access and modify specific columns.
- **Filtering**: Filter rows based on custom criteria.
- **Statistics Calculation**: Compute statistics such as mean, sum, minimum, and maximum for numeric columns.
- **Object Creation**: Create custom objects from CSV data.

## Prerequisites

- C++20 compliant compiler (e.g., `g++` 10.0 or higher)
- CMake (for building the project, optional)

## Files

- `CSVParser.h`: Header file containing the `CSVParser` class definition.
- `CSVParser.cpp`: Implementation file for the `CSVParser` class.
- `class_ex.h`: Header file containing the `Country_Indices` class definition.
- `class_ex.cpp`: Implementation file for the `Country_Indices` class.
- `example/main.cpp`: Example usage of the `CSVParser` class.
- `CLIdx.csv`: Sample CSV file for testing.

## Building the Project

To build the project, navigate to the project directory and use the following commands:

```bash
g++ -std=c++20 -Wall -Wextra -I. -o example_program example/main.cpp
