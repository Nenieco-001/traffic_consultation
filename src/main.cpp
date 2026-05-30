#include <iostream>

#include "io/file_io.h"

int main() {
    auto data = file_io::loadFromFile();
    std::cout << "Loaded " << data.getAllCities().size() << " cities, " << data.getAllTrips().size() << " trips\n";
}