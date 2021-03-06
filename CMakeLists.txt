cmake_minimum_required(VERSION 3.5)

include(cmake/HunterGate.cmake)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.186.tar.gz"
    SHA1 "b215dc4d88cbe6cc74b5cc9fa9cdd7058775c26b"
)

project(class_loader)

hunter_add_package(Boost COMPONENTS thread system)
hunter_add_package(catkin)
hunter_add_package(PocoCpp)
hunter_add_package(ros_console_bridge)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Boost CONFIG REQUIRED COMPONENTS thread system)

set(CATKIN_DISABLED false CACHE BOOL "Disable the catkin build, useful if catkin is present but a build outside of ros is done")
if(NOT CATKIN_DISABLED)
  find_package(catkin QUIET)
endif()

find_package(console_bridge REQUIRED)

if(${catkin_FOUND})
  find_package(Poco REQUIRED Foundation CONFIG)
  set(PROJECT_INSTALLSPACE_LIBRARIES ros::${PROJECT_NAME})
  catkin_package(
    INCLUDE_DIRS include
    LIBRARIES ${PROJECT_NAME}
    DEPENDS "Boost COMPONENTS thread system" "Poco COMPONENTS Foundation CONFIG" console_bridge
    CFG_EXTRAS class_loader-extras.cmake
  )
else()
  message("-- catkin not found")
  set(Poco_DIR cmake)
  find_package(Poco REQUIRED COMPONENTS Foundation)
  set(CATKIN_PACKAGE_LIB_DESTINATION lib)
  set(CATKIN_GLOBAL_BIN_DESTINATION bin)
  set(CATKIN_GLOBAL_BIN_DESTINATION bin)
  set(CATKIN_PACKAGE_INCLUDE_DESTINATION include/class_loader)
endif()

include_directories(include ${console_bridge_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${Poco_INCLUDE_DIRS})

set(${PROJECT_NAME}_SRCS
  src/class_loader.cpp
  src/class_loader_core.cpp
  src/meta_object.cpp
  src/multi_library_class_loader.cpp
)
set(${PROJECT_NAME}_HDRS
  include/class_loader/class_loader.hpp
  include/class_loader/class_loader_core.hpp
  include/class_loader/exceptions.hpp
  include/class_loader/meta_object.hpp
  include/class_loader/multi_library_class_loader.hpp
  include/class_loader/register_macro.hpp
)
add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS} ${${PROJECT_NAME}_HDRS})
target_link_libraries(${PROJECT_NAME} Boost::boost Boost::thread Boost::system ros::console_bridge Poco::Foundation)
if(WIN32)
  # Causes the visibility macros to use dllexport rather than dllimport
  # which is appropriate when building the dll but not consuming it.
  target_compile_definitions(${PROJECT_NAME} PRIVATE "CLASS_LOADER_BUILDING_DLL")
endif()

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION})
install(DIRECTORY include/class_loader/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION})

install(PROGRAMS scripts/class_loader_headers_update.py
  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

install(EXPORT ${PROJECT_NAME}Targets
    DESTINATION share/${PROJECT_NAME}/cmake
    NAMESPACE ros::)

if(CATKIN_ENABLE_TESTING)
  add_subdirectory(test)
endif()

if(NOT catkin_FOUND)
  set(TARGET_NAME ${PROJECT_NAME})
  set(PKGCONFIG_LIBS
    ${Boost_LIBRARIES}
    ${console_bridge_LIBRARIES}
    ${Poco_LIBRARIES}
  )

  # Prepare and install necessary files to support finding of the library
  # using pkg-config
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/${TARGET_NAME}.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.pc @ONLY)

  install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)
endif()
