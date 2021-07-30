cmake_minimum_required(VERSION 3.20)

set(FIREBASE_PYTHON_EXECUTABLE "/Users/dconeybe/local/pyvenv/bin/python" CACHE FILEPATH "The Python interpreter to use, such as one from a venv")

project(ExtDbSyncApp)

set (CMAKE_CXX_STANDARD 11)

# Build a desktop application.
add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)

add_subdirectory("/Users/dconeybe/dev/firebase/firebase_cpp_sdk" bin/ EXCLUDE_FROM_ALL)

add_executable(ExtDbSyncApp ExtDbSyncApp.cc)
target_link_libraries(ExtDbSyncApp
  firebase_firestore
  firebase_auth
  firebase_app
  gssapi_krb5
  pthread
  "-framework CoreFoundation"
  "-framework Foundation"
  "-framework GSS"
  "-framework Security"
  "-framework SystemConfiguration"
)
