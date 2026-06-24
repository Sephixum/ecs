add_executable(ecs_test test.cpp)

target_link_libraries(ecs_test
    PRIVATE ecs
    PRIVATE ecs_sanitizers
)

add_test(NAME ecs_test COMMAND ecs_test)
