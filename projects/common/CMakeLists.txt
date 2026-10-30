add_library(college_ports_common INTERFACE)
target_include_directories(college_ports_common INTERFACE include)
target_link_libraries(college_ports_common INTERFACE
    Cheryl::OpenGL::Startup Cheryl::UI::TGUI)
target_compile_definitions(college_ports_common INTERFACE
    COLLEGE_PORTS_ASSETS="${PROJECT_SOURCE_DIR}/assets")
