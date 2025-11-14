cmake_minimum_required(VERSION 3.16)

project(BarcodeScanner VERSION 2.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# =============================================================================
# Qt6 НАСТРОЙКИ
# =============================================================================

set(CMAKE_PREFIX_PATH "C:/msys64/mingw64")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# =============================================================================
# OpenCV НАСТРОЙКИ
# =============================================================================

set(OpenCV_DIR "C:/msys64/mingw64/lib/cmake/opencv4")
find_package(OpenCV REQUIRED)

# =============================================================================
# ZBar НАСТРОЙКИ
# =============================================================================

set(ZBAR_INCLUDE_DIR "C:/Users/rauko/vcpkg/packages/mchehab-zbar_x64-windows/include")
set(ZBAR_DLL "C:/Users/rauko/vcpkg/packages/mchehab-zbar_x64-windows/bin/zbar-0.dll")

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

# =============================================================================
# ДОПОЛНИТЕЛЬНЫЕ ЗАВИСИМОСТИ
# =============================================================================

set(ICONV_DLL "C:/Users/rauko/vcpkg/packages/libiconv_x64-windows/bin/iconv-2.dll")

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
    set(ZLIB_DLL "C:/msys64/mingw64/bin/zlib1.dll")
    message(STATUS "Using zlib from MSYS2: ${ZLIB_DLL}")
endif()

# =============================================================================
# ИСХОДНЫЕ ФАЙЛЫ ПРОЕКТА
# =============================================================================

qt_add_executable(BarcodeScanner MANUAL_FINALIZATION
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    barcodescanner.cpp
    barcodescanner.h
    country.cpp
    country.h
    manufacturer.cpp
    manufacturer.h
    product.cpp
    product.h
    barcodedetector.cpp
    barcodedetector.h
    barcodedecoder.cpp
    barcodedecoder.h
    barcodevalidator.cpp
    barcodevalidator.h
    barcodepreprocessor.cpp
    barcodepreprocessor.h
    barcoderegionfinder.cpp
    barcoderegionfinder.h
    countrydataloader.cpp
    countrydataloader.h
    manufacturerdataloader.cpp
    manufacturerdataloader.h
    productdataloader.cpp
    productdataloader.h
    fileexportservice.cpp
    fileexportservice.h
    resultdisplay.cpp
    resultdisplay.h
    barcodedetector.h
    barcodedetector.cpp
    barcodedecoder.h
    barcodedecoder.cpp
    barcodevalidator.h
    barcodevalidator.cpp
    barcodepreprocessor.h
    barcodepreprocessor.cpp
    barcoderegionfinder.h
    barcoderegionfinder.cpp
    countrydataloader.h
    countrydataloader.cpp
    manufacturerdataloader.h
    manufacturerdataloader.cpp
    productdataloader.h
    productdataloader.cpp
    fileexportservice.h
    fileexportservice.cpp
    resultdisplay.h
    resultdisplay.cpp
)

# =============================================================================
# НАСТРОЙКИ КОМПИЛЯЦИИ
# =============================================================================

target_compile_definitions(BarcodeScanner PRIVATE
    QT_CORE_LIB
    QT_GUI_LIB
    QT_WIDGETS_LIB
)

target_include_directories(BarcodeScanner PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${ZBAR_INCLUDE_DIR}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
)

# =============================================================================
# НАСТРОЙКИ ЛИНКОВКИ
# =============================================================================

target_link_libraries(BarcodeScanner PRIVATE
    Qt6::Core
    Qt6::Widgets
    ${OpenCV_LIBS}
    "${ZBAR_LIBRARY}"
)

# =============================================================================
# ФУНКЦИЯ ДЛЯ КОПИРОВАНИЯ DLL
# =============================================================================

function(copy_dll_if_exists source target_name)
    if(EXISTS "${source}")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${source}"
                $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copying ${source}"
        )
        message(STATUS "DLL will be copied: ${source}")
    else()
        message(WARNING "DLL not found: ${source}")
    endif()
endfunction()

# =============================================================================
# КОПИРОВАНИЕ ZBar И СИСТЕМНЫХ DLL
# =============================================================================

copy_dll_if_exists("${ZBAR_DLL}" BarcodeScanner)
copy_dll_if_exists("${ICONV_DLL}" BarcodeScanner)
copy_dll_if_exists("${ZLIB_DLL}" BarcodeScanner)

# =============================================================================
# КОПИРОВАНИЕ Qt6 DLL
# =============================================================================

set(QT_BIN_PATH "C:/msys64/mingw64/bin")

message(STATUS "Searching for Qt DLLs in: ${QT_BIN_PATH}")

# Поиск основных Qt DLL
find_file(QT6_CORE_DLL Qt6Core.dll PATHS ${QT_BIN_PATH} NO_DEFAULT_PATH)
find_file(QT6_WIDGETS_DLL Qt6Widgets.dll PATHS ${QT_BIN_PATH} NO_DEFAULT_PATH)
find_file(QT6_GUI_DLL Qt6Gui.dll PATHS ${QT_BIN_PATH} NO_DEFAULT_PATH)

# Проверка найденных DLL
if(QT6_CORE_DLL)
    message(STATUS "✓ Found: ${QT6_CORE_DLL}")
else()
    message(FATAL_ERROR "✗ Not found: Qt6Core.dll")
endif()

if(QT6_WIDGETS_DLL)
    message(STATUS "✓ Found: ${QT6_WIDGETS_DLL}")
else()
    message(FATAL_ERROR "✗ Not found: Qt6Widgets.dll")
endif()

if(QT6_GUI_DLL)
    message(STATUS "✓ Found: ${QT6_GUI_DLL}")
else()
    message(FATAL_ERROR "✗ Not found: Qt6Gui.dll")
endif()

# Копирование основных Qt DLL
copy_dll_if_exists("${QT6_CORE_DLL}" BarcodeScanner)
copy_dll_if_exists("${QT6_WIDGETS_DLL}" BarcodeScanner)
copy_dll_if_exists("${QT6_GUI_DLL}" BarcodeScanner)

# =============================================================================
# КОПИРОВАНИЕ Qt6 ПЛАГИНОВ
# =============================================================================

# Создаем папку для плагинов
add_custom_command(TARGET BarcodeScanner POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:BarcodeScanner>/platforms"
)

# Поиск плагина Windows
find_file(QT6_QWINDOWS_DLL
    NAMES qwindows.dll
    PATHS
        "C:/msys64/mingw64/plugins/platforms"
        "C:/msys64/mingw64/share/qt6/plugins/platforms"
        "C:/msys64/mingw64/lib/qt6/plugins/platforms"
    NO_DEFAULT_PATH
)

if(QT6_QWINDOWS_DLL)
    add_custom_command(TARGET BarcodeScanner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT6_QWINDOWS_DLL}"
                "$<TARGET_FILE_DIR:BarcodeScanner>/platforms/"
    )
    message(STATUS "✓ Platform plugin: ${QT6_QWINDOWS_DLL}")
else()
    message(WARNING "⚠ Platform plugin qwindows.dll not found")
endif()

# =============================================================================
# КОПИРОВАНИЕ ДОПОЛНИТЕЛЬНЫХ Qt ЗАВИСИМОСТЕЙ
# =============================================================================

# ICU библиотеки для интернационализации
file(GLOB ICU_DLLS
    "C:/msys64/mingw64/bin/icudt*.dll"
    "C:/msys64/mingw64/bin/icuin*.dll"
    "C:/msys64/mingw64/bin/icuuc*.dll"
)

foreach(ICU_DLL ${ICU_DLLS})
    copy_dll_if_exists("${ICU_DLL}" BarcodeScanner)
endforeach()

if(ICU_DLLS)
    message(STATUS "✓ ICU libraries copied")
endif()

# =============================================================================
# КОПИРОВАНИЕ OpenCV DLL
# =============================================================================

file(GLOB OPENCV_DLLS "C:/msys64/mingw64/bin/libopencv_*.dll")
foreach(OPENCV_DLL ${OPENCV_DLLS})
    copy_dll_if_exists("${OPENCV_DLL}" BarcodeScanner)
endforeach()

message(STATUS "OpenCV DLLs found: ${OPENCV_DLLS}")

# =============================================================================
# ФИНАЛИЗАЦИЯ Qt
# =============================================================================

qt_finalize_executable(BarcodeScanner)

# =============================================================================
# ИНФОРМАЦИЯ О СБОРКЕ
# =============================================================================

message(STATUS "=== BARCODE SCANNER CONFIGURATION COMPLETE ===")
message(STATUS "Project version: ${BarcodeScanner_VERSION}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Source dir: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Build dir: ${CMAKE_BINARY_DIR}")

# =============================================================================
# ПРОВЕРКА ВСЕХ НЕОБХОДИМЫХ КОМПОНЕНТОВ
# =============================================================================

message(STATUS "=== REQUIRED COMPONENTS ===")
message(STATUS "Qt6 Core: ${Qt6Core_FOUND}")
message(STATUS "Qt6 Widgets: ${Qt6Widgets_FOUND}")
message(STATUS "OpenCV: ${OpenCV_FOUND}")
message(STATUS "ZBar library: ${ZBAR_LIBRARY}")

if(Qt6Core_FOUND AND Qt6Widgets_FOUND AND OpenCV_FOUND AND ZBAR_LIBRARY)
    message(STATUS "✓ All required components found!")
else()
    message(FATAL_ERROR "✗ Some required components are missing!")
endif()
