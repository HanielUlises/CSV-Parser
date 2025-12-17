#include <gtest/gtest.h>
#include "CSVParser.h"
#include <filesystem>

static const std::string TITANIC_CSV = "data/titanic.csv";

TEST(CSVParserBasic, ReadHeaderAndRowCount) {
    CSVParser parser = CSVParser::read_csv(TITANIC_CSV);
    EXPECT_GT(parser.row_count(), 0U);
    EXPECT_GT(parser.column_count(), 0U);

    auto cols = parser.get_columns();
    EXPECT_NE(std::find(cols.begin(), cols.end(), "Survived"), cols.end());
}

TEST(CSVParserBasic, GetColumn) {
    CSVParser parser = CSVParser::read_csv(TITANIC_CSV);
    auto ages = parser.get_column("Age");
    EXPECT_EQ(ages.size(), parser.row_count());
}

TEST(CSVParserStats, MeanAgePositive) {
    CSVParser parser = CSVParser::read_csv(TITANIC_CSV);
    double avgAge = parser.mean("Age");
    EXPECT_GT(avgAge, 0.0);
}

TEST(CSVParserFilter, FilterSurvived) {
    CSVParser parser = CSVParser::read_csv(TITANIC_CSV);
    auto survivors = parser.filter([](auto &row) {
        return row[0] == "1";
    });
    EXPECT_LE(survivors.row_count(), parser.row_count());
}
