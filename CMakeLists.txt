cmake_minimum_required(VERSION 3.10)
     project(OBSNLController LANGUAGES CXX)

     set(CMAKE_CXX_STANDARD 17)
     set(CMAKE_CXX_STANDARD_REQUIRED ON)

     find_package(websocketpp REQUIRED)
     find_package(nlohmann_json REQUIRED)
     find_package(Boost REQUIRED COMPONENTS system)
     find_package(OpenSSL REQUIRED)

     include_directories(${CMAKE_SOURCE_DIR}/include)

     add_executable(obs-nl-controller
         src/main.cpp
         src/obs_controller.cpp
         src/nlp_parser.cpp
     )

     target_link_libraries(obs-nl-controller PRIVATE websocketpp::websocketpp nlohmann_json::nlohmann_json Boost::system OpenSSL::Crypto)