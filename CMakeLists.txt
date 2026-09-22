cmake_minimum_required(VERSION 3.22.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(mideej)

target_sources(
  app
  PRIVATE
  src/main.c
  src/usb_setup.c
  src/midi_setup.c
)

target_include_directories(app PRIVATE
  inc
  src
)
