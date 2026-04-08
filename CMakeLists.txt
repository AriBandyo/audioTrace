cmake_minimum_required(VERSION 3.15)
project(AudioTrace)
set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)
pkg_check_modules(FFTW REQUIRED fftw3)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

find_library(LIBPQ_LIB pq PATHS /c/msys64/mingw64/lib)
include_directories(/c/msys64/mingw64/include)

add_executable(audiotrace main.cpp)
target_include_directories(audiotrace PRIVATE ${SNDFILE_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
target_link_libraries(audiotrace PRIVATE ${SNDFILE_LIBRARIES} ${FFTW_LIBRARIES} ${LIBPQ_LIB})

add_executable(match match.cpp)
target_include_directories(match PRIVATE ${SNDFILE_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
target_link_libraries(match PRIVATE ${SNDFILE_LIBRARIES} ${FFTW_LIBRARIES} ${LIBPQ_LIB})

add_executable(listen listen.cpp)
target_include_directories(listen PRIVATE ${PORTAUDIO_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
target_link_libraries(listen PRIVATE ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES} ${LIBPQ_LIB})