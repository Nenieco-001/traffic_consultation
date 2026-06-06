#pragma once

#include <string>

#include "domain/data/transport_data.h"
#include "domain/model/query_request.h"

// 应用层编排器：Menu 通过它访问领域层和数据层
class ConsultController {
  public:
    ConsultController() = default;

    QueryResult query(const QueryRequest& req);  // 按 strategy 分发到三种算法
    void loadData(const std::string& dir = "");  // 委托 file_io 加载
    void saveData(const std::string& dir = "");  // 委托 file_io 保存

    // CRUD 直接委托给 TransportData
    void addCity(const City& city);
    void removeCity(int city_id);
    void addTrip(const Trip& trip);
    void removeTrip(int trip_id);
    bool modifyTrip(int id, const Trip& new_data);  // 修改班次，返回是否找到并更新

    const TransportData& getData() const { return data_; }  // 供 Menu 渲染用

  private:
    TransportData data_;
    City findCityById(int id) const;
};
