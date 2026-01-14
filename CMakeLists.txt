cmake_minimum_required(VERSION 3.15)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Toy2D
        LANGUAGES CXX
        DESCRIPTION "a toy 2d renderer made in vulkan")

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用位置无关代码（对静态库重要）
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# 包含自定义模块
include(cmake/FindVulkan.cmake)
include(cmake/CopyFiles.cmake)

# 查找glslc编译器
find_program(GLSLC_PROGRAM glslc REQUIRED)

# 编译shader
message(STATUS "run glslc to compile shaders ...")
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.vert -o ${CMAKE_BINARY_DIR}/vert.spv)
execute_process(COMMAND ${GLSLC_PROGRAM} ${CMAKE_SOURCE_DIR}/shader/shader.frag -o ${CMAKE_BINARY_DIR}/frag.spv)
message(STATUS "compile shader OK")

# 收集源文件
aux_source_directory(src SRC)

# 添加toy2d静态库
add_library(toy2d STATIC ${SRC})

# 设置包含目录
target_include_directories(toy2d PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}  # 包含编译生成的shader
)

# 配置SDL2
# 方法1：如果SDL2是预编译的库
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/SDL2")
    # 设置SDL2路径
    set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rd_party/SDL2")
    
    # 添加包含目录
    target_include_directories(toy2d PUBLIC 
        "${SDL2_DIR}/include"
    )
    
    # 根据平台设置库路径
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)  # 64位
            target_link_directories(toy2d PUBLIC "${SDL2_DIR}/lib/x64")
        else()  # 32位
            target_link_directories(toy2d PUBLIC "${SDL2_DIR}/lib/x86")
        endif()
    else()
        target_link_directories(toy2d PUBLIC "${SDL2_DIR}/lib")
    endif()
    
    # 链接SDL2库
    if(WIN32)
        # Windows需要先链接SDL2main，再链接SDL2
        target_link_libraries(toy2d PRIVATE SDL2main SDL2)
    else()
        target_link_libraries(toy2d PRIVATE SDL2)
    endif()
    
else()
    # 方法2：通过find_package查找系统安装的SDL2
    find_package(SDL2 REQUIRED)
    target_link_libraries(toy2d PRIVATE SDL2::SDL2)
endif()

# 链接Vulkan
target_link_libraries(toy2d PRIVATE Vulkan::Vulkan)

# 设置编译特性
target_compile_features(toy2d PRIVATE cxx_std_17)

# 添加sandbox子目录
add_subdirectory(sandbox)