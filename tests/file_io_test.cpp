#include "io/file_io.h"
#include <cassert>

int main() {
    // 测试写入
    {
        std::vector<City> cities = {{1, "CityA"}, {2, "CityB"}};
        // 注意：这里的时间字段使用绝对分钟数，价格字段用整数表示
        std::vector<Trip> trips = {
            {1, TransportType::TRAIN, 1, 2, 480, 600, 100, "T123"},  // 08:00 - 10:00
            {2, TransportType::PLANE, 1, 2, 540, 660, 300, "P456"}   // 09:00 - 11:00
        };
        std::string customDir = std::string(PROJECT_ROOT) + "/tests_data/file_io_test";
        file_io::saveToFile(cities, trips, customDir);
    }

    // 测试读取
    {
        std::string customDir = std::string(PROJECT_ROOT) + "/tests_data/file_io_test";
        auto data = file_io::loadFromFile(customDir);

        auto cities = data.getAllCities();
        auto trips = data.getAllTrips();
        // 断言检查读取的数据是否正确
        assert(cities.size() == 2);
        assert(cities[0].id_ == 1 && cities[0].name_ == "CityA");
        assert(cities[1].id_ == 2 && cities[1].name_ == "CityB");
        assert(trips.size() == 2);
        assert(trips[0].id_ == 1 && trips[0].type_ == TransportType::TRAIN &&
               trips[0].from_city_id_ == 1 && trips[0].to_city_id_ == 2 &&
               trips[0].departure_time_ == 480 && trips[0].arrival_time_ == 600 &&
               trips[0].price_ == 100 && trips[0].trip_number_ == "T123");
        assert(trips[1].id_ == 2 && trips[1].type_ == TransportType::PLANE &&
               trips[1].from_city_id_ == 1 && trips[1].to_city_id_ == 2 &&
               trips[1].departure_time_ == 540 && trips[1].arrival_time_ == 660 &&
               trips[1].price_ == 300 && trips[1].trip_number_ == "P456");
        std::printf("File I/O tests passed successfully.\n");
    }

    return 0;
}
