cmake_minimum_required(VERSION 3.5)
project(interneuron_lib)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(include)

if(${INTERNEURON})
  add_compile_options(-DINTERNEURON -fPIC)
endif()
if(${PRINT_DEBUG})
  add_compile_options(-DPRINT_DEBUG)
endif()

find_package(ament_cmake REQUIRED)
#find_package(rclcpp REQUIRED)
#find_package(interneuron_msgs REQUIRED)

set(SOURCES
  "src/time_point_manager.cpp"
)

add_library(${PROJECT_NAME} ${SOURCES})

#ament_target_dependencies(${PROJECT_NAME} interneuron_msgs)
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}/
)
install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  #INCLUDES DESTINATION include
)


ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})

ament_package()
