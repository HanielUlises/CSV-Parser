#include "../CSVParser.h"

/*
    This class example is for creating objects with the CSV data, some may find this useful
*/
class Country_Indices {
public:
    int rank;
    std::string name;
    double cost;
    double rent;
    double cost_rent;
    double groceries;
    double restaurant_prices;
    double local_purchasing;

    // Constructor using parameter packing
    Country_Indices(int r, std::string n, double c, double re, double cr, double g, double rp, double lp)
        : rank(r), name(n), cost(c), rent(re), cost_rent(cr), groceries(g), restaurant_prices(rp), local_purchasing(lp) {}
};
