add_library(spqb_core STATIC
    version.cpp
)

target_include_directories(spqb_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(spqb_core PUBLIC cxx_std_20)
