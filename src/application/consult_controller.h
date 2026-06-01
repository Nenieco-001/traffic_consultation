#pragma once

#include <string>

#include "domain/model/query_request.h"
#include "domain/data/transport_data.h"
// TODO(cleanup): consult_controller.cpp 使用了 algo::xxx，该依赖放在 .cpp 中而非头文件

class ConsultController {
  public:
    ConsultController() = default;

    // 查询接口
    QueryResult query(const QueryRequest& req);

    // 数据持久化
    void loadData(const std::string& dir = "");
    void saveData(const std::string& dir = "");

    // 数据写操作
    void addCity(const City& city);
    void removeCity(int city_id);
    void addTrip(const Trip& trip);
    void removeTrip(int trip_id);

    // 只读访问（供 Menu 展示用）
    const TransportData& getData() const { return data_; }

  private:
    TransportData data_;
    City findCityById(int id) const;
};
