cmake_minimum_required(VERSION 3.19)
project(AP_PROJECT_PLATWO LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_resources(RESOURCES
    "resources/resources.qrc"
)

qt_add_executable(AP_PROJECT_PLATWO
    WIN32 MACOSX_BUNDLE

    main.cpp

    src/Ui/Loginwindow.cpp
    src/Ui/Loginwindow.h
    src/Ui/Loginwindow.ui
    src/Ui/loginpage.h src/Ui/loginpage.cpp src/Ui/loginpage.ui

    ${RESOURCES}
    src/Ui/signuppage.h src/Ui/signuppage.cpp src/Ui/signuppage.ui
    src/Ui/forgotpasspage.h src/Ui/forgotpasspage.cpp src/Ui/forgotpasspage.ui


)

target_include_directories(AP_PROJECT_PLATWO PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Ui
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/qss
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/images


)

target_link_libraries(AP_PROJECT_PLATWO
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS AP_PROJECT_PLATWO
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET AP_PROJECT_PLATWO
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
