#pragma once
#include <vector>

#include "domain/model/domain_types.h"

// 领域层数据仓库：城市和班次的内存存储，提供 CRUD + 查询
class TransportData {
  private:
    std::vector<City> cities_;
    std::vector<Trip> trips_;

  public:
    TransportData() = default;

    void addCity(const City& city);
    void removeCity(int city_id);  // 级联删除关联班次（std::erase_if）
    void addTrip(const Trip& trip);
    void removeTrip(int trip_id);
    bool updateTrip(int id, const Trip& new_data);  // 更新班次：找到 id 则替换返回 true，否则返回 false
    std::vector<Trip> getDeparturesInWindow(int from_city_id, int from_time, int to_time, TransportType type) const;
    std::vector<Trip> getTripsByCity(int city_id) const;  // 查出关联班次（不删），用于级联警告

    const std::vector<City>& getAllCities() const;
    const std::vector<Trip>& getAllTrips() const;
    int maxCityId() const;  // 最大城市编号，用于算法层状态数组的 size 推算
};
