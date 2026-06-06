cmake_minimum_required(VERSION 3.25)
project(Traffic_Information LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions("PROJECT_ROOT=\"${CMAKE_SOURCE_DIR}\"")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
include_directories(${CMAKE_SOURCE_DIR}/inc ${CMAKE_SOURCE_DIR}/src)

add_executable(Traffic_Information
    src/presentation/main.cpp
    src/domain/tools/time_process.cpp
    src/domain/algorithm/dijkstra.cpp
    src/domain/data/transport_data.cpp
    src/domain/data/user_repository.cpp
    src/infrastructure/file_io.cpp
    src/infrastructure/file_config.cpp
    src/application/consult_controller.cpp
    src/application/auth_service.cpp
    src/presentation/menu.cpp
)

add_executable(time_process_test
    tests/time_process_test.cpp
    src/domain/tools/time_process.cpp
)

add_executable(transport_type_test
    tests/transport_type_test.cpp
)

add_executable(query_request_test
    tests/query_request_test.cpp
)

add_executable(file_io_test
    tests/file_io_test.cpp
    src/domain/tools/time_process.cpp
    src/domain/data/transport_data.cpp
    src/infrastructure/file_io.cpp
    src/infrastructure/file_config.cpp
)

add_executable(data_layer_test
    tests/data_layer_test.cpp
    src/domain/data/transport_data.cpp
    src/infrastructure/file_io.cpp
    src/infrastructure/file_config.cpp
)

add_executable(algo_layer_test
    tests/algo_layer_test.cpp
    src/domain/data/transport_data.cpp
    src/infrastructure/file_io.cpp
    src/infrastructure/file_config.cpp
    src/domain/algorithm/dijkstra.cpp
)

add_executable(controller_test
    tests/controller_test.cpp
    src/domain/tools/time_process.cpp
    src/domain/algorithm/dijkstra.cpp
    src/domain/data/transport_data.cpp
    src/infrastructure/file_io.cpp
    src/infrastructure/file_config.cpp
    src/application/consult_controller.cpp
)

add_executable(integration_test
    tests/integration_test.cpp
    src/domain/tools/time_process.cpp
    src/domain/algorithm/dijkstra.cpp
    src/domain/data/transport_data.cpp
    src/domain/data/user_repository.cpp
    src/infrastructure/file_io.cpp
    src/infrastructure/file_config.cpp
    src/application/consult_controller.cpp
)
