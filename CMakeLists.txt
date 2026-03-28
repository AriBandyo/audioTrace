cmake_minimum_required(VERSION 3.15)
project(AudioTrace)
set(CMAKE_CXX_STANDARD 17)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)
pkg_check_modules(FFTW REQUIRED fftw3)
add_executable(audiotrace main.cpp)
target_include_directories(audiotrace PRIVATE ${SNDFILE_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
target_link_libraries(audiotrace PRIVATE ${SNDFILE_LIBRARIES} ${FFTW_LIBRARIES})



add_executable(match match.cpp)
target_include_directories(match PRIVATE ${SNDFILE_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
target_link_libraries(match PRIVATE ${SNDFILE_LIBRARIES} ${FFTW_LIBRARIES})



pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

add_executable(listen listen.cpp)
target_include_directories(listen PRIVATE ${PORTAUDIO_INCLUDE_DIRS} ${FFTW_INCLUDE_DIRS})
target_link_libraries(listen PRIVATE ${PORTAUDIO_LIBRARIES} ${FFTW_LIBRARIES})