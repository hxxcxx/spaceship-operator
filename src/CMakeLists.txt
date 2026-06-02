add_executable(spaceship-operator
    main.cpp
)

target_include_directories(spaceship-operator PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)
