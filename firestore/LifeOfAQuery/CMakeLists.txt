# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.8)

set(FIREBASE_PYTHON_EXECUTABLE "python" CACHE FILEPATH
  "The Python interpreter to use, such as one from a venv")

# User settings for Firebase integration tests.
# Path to Firebase SDK.
# Try to read the path to the Firebase C++ SDK from an environment variable.
if (NOT "$ENV{FIREBASE_CPP_SDK_DIR}" STREQUAL "")
  set(DEFAULT_FIREBASE_CPP_SDK_DIR "$ENV{FIREBASE_CPP_SDK_DIR}")
else()
  if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../cpp_sdk_version.json")
    set(DEFAULT_FIREBASE_CPP_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")
  else()
    set(DEFAULT_FIREBASE_CPP_SDK_DIR "firebase_cpp_sdk")
  endif()
endif()
if ("${FIREBASE_CPP_SDK_DIR}" STREQUAL "")
  set(FIREBASE_CPP_SDK_DIR ${DEFAULT_FIREBASE_CPP_SDK_DIR})
endif()
if(NOT EXISTS ${FIREBASE_CPP_SDK_DIR})
  message(FATAL_ERROR "The Firebase C++ SDK directory does not exist: ${FIREBASE_CPP_SDK_DIR}. See the readme.md for more information")
endif()

# Windows runtime mode, either MD or MT depending on whether you are using
# /MD or /MT. For more information see:
# https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx
set(MSVC_RUNTIME_MODE MD)

project(LifeOfAQuery)
set (CMAKE_CXX_STANDARD 11)

# Build a desktop application.
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Windows runtime mode, either MD or MT depending on whether you are using
# /MD or /MT. For more information see:
# https://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx
set(MSVC_RUNTIME_MODE MD)

add_executable(LifeOfAQuery main.cc)

if(APPLE)
  set(ADDITIONAL_LIBS
    gssapi_krb5
    pthread
    "-framework CoreFoundation"
    "-framework Foundation"
    "-framework GSS"
    "-framework Security"
    "-framework SystemConfiguration"
  )
elseif(MSVC)
  set(ADDITIONAL_LIBS advapi32 ws2_32 crypt32)
else()
  set(ADDITIONAL_LIBS pthread)
endif()

# Add the Firebase libraries to the target using the function from the SDK.
add_subdirectory(${FIREBASE_CPP_SDK_DIR} bin/ EXCLUDE_FROM_ALL)
# Note that firebase_app needs to be last in the list.

target_link_libraries(LifeOfAQuery firebase_firestore firebase_auth firebase_app ${ADDITIONAL_LIBS})
