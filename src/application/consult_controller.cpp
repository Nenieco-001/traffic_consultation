#include "consult_controller.h"

#include <stdexcept>
#include <utility>  // std::move

#include "domain/algorithm/dijkstra.h"
#include "infrastructure/file_io.h"

void ConsultController::loadData(const std::string& dir) {
    data_ = file_io::loadFromFile(dir);
}

void ConsultController::saveData(const std::string& dir) {
    file_io::saveToFile(data_.getAllCities(), data_.getAllTrips(), dir);
}

void ConsultController::addCity(const City& city) {
    data_.addCity(city);
}
void ConsultController::removeCity(int city_id) {
    data_.removeCity(city_id);
}
void ConsultController::addTrip(const Trip& trip) {
    data_.addTrip(trip);
}
void ConsultController::removeTrip(int trip_id) {
    data_.removeTrip(trip_id);
}
bool ConsultController::modifyTrip(int id, const Trip& new_data) {
    return data_.updateTrip(id, new_data);
}

// 策略模式：根据 req.strategy_ 选择不同的 Dijkstra 变体
QueryResult ConsultController::query(const QueryRequest& req) {
    City from = findCityById(req.from_city_id_);
    City to = findCityById(req.to_city_id_);

    Path path;
    switch (req.strategy_) {
        case Strategy::FASTEST: path = algo::findFastestPath(data_, from, to, req.depart_after_, req.transport_); break;
        case Strategy::CHEAPEST:
            path = algo::findCheapestPath(data_, from, to, req.depart_after_, req.transport_);
            break;
        case Strategy::LEAST_TRANSFERS:
            path = algo::findLeastTransferPath(data_, from, to, req.depart_after_, req.transport_);
            break;
        default: throw std::invalid_argument("无效的查询策略");
    }

    QueryResult result;
    if (!path.segments_.empty()) {
        result.paths.push_back(std::move(path));
    } else {
        result.error_msg = "未找到可行路径";
    }
    return result;
}

City ConsultController::findCityById(int id) const {
    for (const auto& city : data_.getAllCities()) {
        if (city.id_ == id)
            return city;
    }
    throw std::invalid_argument("未找到城市 (ID: " + std::to_string(id) + ")");
}
