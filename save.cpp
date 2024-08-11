#include <iostream>
#include <vector>
#include <string>
#include <fstream>

std::vector<std::string> tokenise(std::string csvLine, char separator){
    std::vector<std::string> tokens;
    signed int start, end;
    std::string token;
    start = csvLine.find_first_not_of(separator, 0);
    do
    {
        end = csvLine.find_first_of(separator, start);
        if (start == csvLine.length() || start == end)
            break;
        if (end >= 0)
            token = csvLine.substr(start, end - start);
        else
            token = csvLine.substr(start, csvLine.length() - start);
        tokens.push_back(token);
        start = end + 1;
        //}while(end > 0);
        // slight change from the video -
        // find_first_of returns std::string::npos
        // not 'less than zero'
    } while (end != std::string::npos);
    return tokens;
}

int main()
{
    std::string csvFilename{"CLIdx.csv"};
    std::ifstream csvFile{csvFilename};
    std::string line;
    std::vector<std::vector<std::string>> total;
    std::vector<std::string> tokens;

    while (std::getline(csvFile, line))
    {
        tokens = tokenise(line, ',');
        total.push_back(tokens);
    }

    if (!total.empty()){
        total.erase(total.begin());
    }

    for (const auto &row : total)
    {
        for (const auto &cell : row)
        {
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
    csvFile.close();
    return 0;
}
