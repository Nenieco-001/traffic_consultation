#pragma once
#include <vector>

#include "domain/model/domain_types.h"

class TransportData {
  private:
    std::vector<City> cities_;  // 存储城市信息
    std::vector<Trip> trips_;   // 存储班次信息
  public:
    // 构造函数
    TransportData() = default;
    // 城市添加
    void addCity(const City& city);
    // 城市删除
    void removeCity(int city_id);
    // 班次添加
    void addTrip(const Trip& trip);
    // 班次删除
    void removeTrip(int trip_id);
    // 获取出发时间窗口内的班次 - 按出发时间排序
    std::vector<Trip> getDeparturesInWindow(int from_city_id, int from_time, int to_time, TransportType type) const;
    // 获取所有城市列表
    const std::vector<City>& getAllCities() const;
    // 获取所有班次列表
    const std::vector<Trip>& getAllTrips() const;
};