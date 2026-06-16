#include "domain/data/transport_data.h"

#include <algorithm>

void TransportData::addCity(const City& city) {
    cities_.push_back(city);
}

// C++20 std::erase_if：删除城市 + 该城市出发或到达的所有班次
void TransportData::removeCity(int id) {
    std::erase_if(cities_, [id](const City& city) { return city.id_ == id; });
    std::erase_if(trips_, [id](const Trip& trip) { return trip.from_city_id_ == id || trip.to_city_id_ == id; });
}

void TransportData::addTrip(const Trip& trip) {
    trips_.push_back(trip);
}

void TransportData::removeTrip(int id) {
    std::erase_if(trips_, [id](const Trip& trip) { return trip.id_ == id; });
}

bool TransportData::updateTrip(int id, const Trip& new_data) {
    for (auto& trip : trips_) {
        if (trip.id_ == id) {
            trip = new_data;
            return true;
        }
    }
    return false;
}

// 窗口查询：按出发时间排序返回，type == MIXED 时不过滤交通工具
std::vector<Trip> TransportData::getDeparturesInWindow(int from_city_id, int from_time, int to_time,
                                                       TransportType type) const {
    std::vector<Trip> result;
    for (const auto& trip : trips_) {
        if (trip.from_city_id_ == from_city_id && trip.departure_time_ >= from_time &&
            trip.departure_time_ <= to_time && (type == TransportType::MIXED || trip.type_ == type)) {
            result.push_back(trip);
        }
    }
    std::sort(result.begin(), result.end(),
              [](const Trip& a, const Trip& b) { return a.departure_time_ < b.departure_time_; });
    return result;
}

// 查出与指定城市关联的班次（不删除），供 Menu 级联警告展示用
std::vector<Trip> TransportData::getTripsByCity(int city_id) const {
    std::vector<Trip> result;
    for (const auto& trip : trips_) {
        if (trip.from_city_id_ == city_id || trip.to_city_id_ == city_id)
            result.push_back(trip);
    }
    return result;
}

const std::vector<City>& TransportData::getAllCities() const {
    return cities_;
}
const std::vector<Trip>& TransportData::getAllTrips() const {
    return trips_;
}

int TransportData::maxCityId() const {
    int max_id = 0;
    for (const auto& c : cities_)
        if (c.id_ > max_id) max_id = c.id_;
    return max_id;
}
