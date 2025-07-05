cmake_minimum_required(VERSION 3.10)
project(OBSNLController LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's meta-object compiler (moc) and UI compiler (uic)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find Qt5 packages
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# Find other dependencies
find_package(websocketpp REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/ui)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENSSL_INCLUDE_DIR})

# Source files
set(SOURCES
    src/main.cpp
    src/obs_controller.cpp
    src/nlp_parser.cpp
    ui/mainwindow.cpp
)

# Header files for moc (explicitly include to ensure AUTOMOC processes them)
set(HEADERS
    include/obs_controller.hpp
    ui/mainwindow.hpp
)

# UI files
set(UI_FILES
    ui/mainwindow.ui
)

# Add executable
add_executable(obs-nl-controller ${SOURCES} ${HEADERS} ${UI_FILES})

# Link libraries
target_link_libraries(obs-nl-controller PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    websocketpp::websocketpp
    nlohmann_json::nlohmann_json
    Boost::system
    OpenSSL::Crypto
)

# Set UI directory for AUTOUIC
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)