#include "data/transport_data.h"

#include <algorithm>

void TransportData::addCity(const City& city) {
    // TODO(optimize): 若已知数据量上限，可 cities_.reserve() 预分配内存以避免多次扩容
    cities_.push_back(city);
}
void TransportData::removeCity(int id) {
    // TODO: 删除城市前，需要检查是否有班次经过该城市，并处理相关班次（删除或标记为无效）
    std::erase_if(cities_, [id](const City& city) { return city.id_ == id; });  // C++20 的 erase_if
}
void TransportData::addTrip(const Trip& trip) {
    // TODO(optimize): 同 addCity，可预分配内存
    trips_.push_back(trip);
}
void TransportData::removeTrip(int id) {
    std::erase_if(trips_, [id](const Trip& trip) { return trip.id_ == id; });  // C++20 的 erase_if
}
std::vector<Trip> TransportData::getDeparturesInWindow(int from_city_id, int from_time, int to_time, TransportType type) const {
    std::vector<Trip> result;
    for (const auto& trip : trips_) {
        if (trip.from_city_id_ == from_city_id && trip.type_ == type && trip.departure_time_ >= from_time &&
            trip.departure_time_ <= to_time) {
            result.push_back(trip);
        }
    }
    std::sort(result.begin(), result.end(),
              [](const Trip& a, const Trip& b) { return a.departure_time_ < b.departure_time_; });
    return result;
}
std::vector<Trip> TransportData::getDeparturesAfter(int from_city_id, int earliest_time, TransportType type) const {
    std::vector<Trip> result;
    for (const auto& trip : trips_) {
        if (trip.from_city_id_ == from_city_id && trip.type_ == type && trip.departure_time_ >= earliest_time) {
            result.push_back(trip);
        }
    }
    std::sort(result.begin(), result.end(),
              [](const Trip& a, const Trip& b) { return a.departure_time_ < b.departure_time_; });
    return result;
}
const std::vector<City>& TransportData::getAllCities() const {
    return cities_;
}
const std::vector<Trip>& TransportData::getAllTrips() const {
    return trips_;
}