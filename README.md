# CSVParser

A lightweight, header-only C++ library for parsing, validating, and processing CSV (Comma-Separated Values) files.

CSVParser provides a strict, row-oriented CSV parser with optional header support, robust structural validation, and a small set of higher-level operations for transforming data, computing statistics, and constructing user-defined objects.

---

## Features

- RFC-style CSV parsing with quoted fields and escaped quotes
- Strict row width and schema validation
- Optional header parsing with column name lookup
- Row-wise filtering and in-place column transformation
- Numeric statistics: sum, mean, min, max
- Construction of user-defined objects from CSV rows
- Header-only, dependency-free design

---

## Requirements

- C++17 or newer
- Standard library only
- CMake 3.16+ (optional)

---

## Installation

Include the header and add the include path:

```cpp
#include <csv/CSVParser.h>
```

---

## Basic Usage

```cpp
auto csv = CSVParser::read_csv("data.csv");

csv.print();

auto filtered = csv.filter([](const auto& row) {
    return std::stod(row[2]) > 10.0;
});

double avg = csv.mean("price");
```

---

## Object Construction

```cpp
struct Person {
    std::string name;
    int age;
};

auto people = csv.create_objects<Person, std::string, int>();
```

Rows are mapped positionally to constructor arguments.

---

## License

MIT
