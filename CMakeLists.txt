cmake_minimum_required(VERSION 3.16)

project(BarcodeScanner VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# OpenCV из MSYS2
set(OpenCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)

# ZBar пути
set(ZBAR_INCLUDE_DIR "C:/Users/rauko/vcpkg/packages/mchehab-zbar_x64-windows/include")
set(ZBAR_DLL "C:/Users/rauko/vcpkg/packages/mchehab-zbar_x64-windows/bin/zbar-0.dll")

# 🔥 АВТОМАТИЧЕСКИЙ ПОИСК БИБЛИОТЕКИ ZBAR
file(GLOB ZBAR_LIBRARIES
    "C:/Users/rauko/vcpkg/packages/mchehab-zbar_x64-windows/lib/*.lib"
    "C:/Users/rauko/vcpkg/packages/mchehab-zbar_x64-windows/lib/*.a"
)

if(ZBAR_LIBRARIES)
    list(GET ZBAR_LIBRARIES 0 ZBAR_LIBRARY)
    message(STATUS "Found ZBar library: ${ZBAR_LIBRARY}")
else()
    message(FATAL_ERROR "No ZBar library found! Check vcpkg installation.")
endif()

# Зависимости
set(ICONV_DLL "C:/Users/rauko/vcpkg/packages/libiconv_x64-windows/bin/iconv-2.dll")

# Поиск zlib в vcpkg
find_file(ZLIB_DLL
    NAMES zlib1.dll
    PATHS
        "C:/Users/rauko/vcpkg/packages/*zlib*/bin"
        "C:/Users/rauko/vcpkg/installed/x64-windows/bin"
    NO_DEFAULT_PATH
)

if(ZLIB_DLL)
    message(STATUS "Found zlib: ${ZLIB_DLL}")
else()
    # Используем zlib из MSYS2
    set(ZLIB_DLL "C:/msys64/mingw64/bin/zlib1.dll")
    message(STATUS "Using zlib from MSYS2: ${ZLIB_DLL}")
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(BarcodeScanner MANUAL_FINALIZATION
    main.cpp
    mainwindow.cpp
    mainwindow.h
    barcodescanner.cpp
    barcodescanner.h
)

# 🔥 ДОБАВЛЯЕМ ЛИНКОВКУ С БИБЛИОТЕКОЙ ZBAR
target_link_libraries(BarcodeScanner PRIVATE
    Qt6::Widgets
    ${OpenCV_LIBS}
    "${ZBAR_LIBRARY}"  # 🔥 ВАЖНО: добавляем библиотеку
)

target_include_directories(BarcodeScanner PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${ZBAR_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Функция для копирования DLL
function(copy_dll_if_exists source)
    if(EXISTS "${source}")
        add_custom_command(TARGET BarcodeScanner POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${source}"
                $<TARGET_FILE_DIR:BarcodeScanner>
        )
        message(STATUS "DLL copied: ${source}")
    else()
        message(WARNING "DLL not found: ${source}")
    endif()
endfunction()

# Копируем DLL
copy_dll_if_exists("${ZBAR_DLL}")
copy_dll_if_exists("${ICONV_DLL}")
copy_dll_if_exists("${ZLIB_DLL}")

qt_finalize_executable(BarcodeScanner)
