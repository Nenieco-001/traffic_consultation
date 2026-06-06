#pragma once
#include "domain/data/transport_data.h"


namespace algo {
    // 最快到达路径
    Path findFastestPath(const TransportData& data, City from_city, City to_city, int depart_time,
                         TransportType transport_type);
    // 最省钱路径
    Path findCheapestPath(const TransportData& data, City from_city, City to_city, int depart_time,
                          TransportType transport_type);
    // 最少换乘路径
    Path findLeastTransferPath(const TransportData& data, City from_city, City to_city, int depart_time,
                               TransportType transport_type);
}  // namespace algo