cmake_minimum_required(VERSION 3.15.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -DBOOST_ASIO_HAS_STD_COROUTINE -DBOOST_ASIO_HAS_CO_AWAIT")

project(web_engine VERSION 0.0.1 LANGUAGES CXX)

set(fmt_DIR ${CMAKE_CURRENT_BINARY_DIR}/../fmt)
add_definitions(-DFMT_HEADER_ONLY)

find_package(Boost 1.72.0 COMPONENTS system
  filesystem # required by Boost.DLL
  REQUIRED)
find_package(Threads)
find_package(fmt)
find_package(OpenSSL)

add_executable(
  ${PROJECT_NAME}

  main.cpp
)

target_link_libraries(
  ${PROJECT_NAME}

  PRIVATE

  Boost::system
  Boost::filesystem

  Threads::Threads
  fmt::fmt-header-only

  OpenSSL::SSL

  ${CMAKE_DL_LIBS}
)

set_target_properties(
  ${PROJECT_NAME}

  PROPERTIES

  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_compile_definitions(
  ${PROJECT_NAME}

  PRIVATE

  BOOST_BEAST_USE_STD_STRING_VIEW
)

# posts plugin part
add_library(
  posts
  MODULE
  view_plugins/posts_view_plugin.cpp
)

set_target_properties(
  posts

  PROPERTIES

  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(
  posts

  PRIVATE

  fmt::fmt-header-only
  Boost::system
)

# about plugin part
add_library(
  about
  MODULE
  view_plugins/about_view_plugin.cpp
)

set_target_properties(
  about

  PROPERTIES

  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(
  about

  PRIVATE

  fmt::fmt-header-only
  Boost::system
)

# contact plugin part
add_library(
  contact
  MODULE
  view_plugins/contact_view_plugin.cpp
)

set_target_properties(
  contact

  PROPERTIES

  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(
  contact

  PRIVATE

  fmt::fmt-header-only
  Boost::system
)

# Editor_project_save_view_plugin

add_library(
  editor_project_save_view_plugin
  MODULE
  view_plugins/editor_project_save_view_plugin.cpp
)

set_target_properties(
  editor_project_save_view_plugin

  PROPERTIES

  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(
  editor_project_save_view_plugin

  PRIVATE

  fmt::fmt-header-only
  Boost::system
)

# Editor_project_load_view_plugin

add_library(
  editor_project_load_view_plugin
  MODULE
  view_plugins/editor_project_load_view_plugin.cpp
)

set_target_properties(
  editor_project_load_view_plugin

  PROPERTIES

  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

target_link_libraries(
  editor_project_load_view_plugin

  PRIVATE

  fmt::fmt-header-only
  Boost::system
)
