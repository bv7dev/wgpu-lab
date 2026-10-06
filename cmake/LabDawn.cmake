# Provides the imported target dawn::webgpu_dawn.
#
# wgpu-lab is pinned to one Dawn release. To move to a newer one, pick a tag from
# https://github.com/google/dawn/releases, then update LAB_DAWN_TAG, LAB_DAWN_SHA
# (the commit in the archive names) and the archive hashes below (the "sha256"
# digests shown next to each release asset).

set(LAB_DAWN_TAG "v20261002.154047")
set(LAB_DAWN_SHA "b1236a9bb4a47f1262bc3acb253bb1c2181e90dd")

set(LAB_DAWN "prebuilt" CACHE STRING
  "Where Dawn comes from: prebuilt (release archive), source (build it), system (find_package)")
set_property(CACHE LAB_DAWN PROPERTY STRINGS prebuilt source system)

include(FetchContent)
find_package(Threads REQUIRED)

# prebuilt: release archives published by Dawn's CI ------------------------------
# Returns the archive name suffix and hash for the host, or leaves them empty if
# Dawn publishes no archive for it.
function(lab_dawn_prebuilt_archive out_name out_hash)
  set(name "")
  set(hash "")
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    # no C runtime flavours on Linux: the small Release archive serves every build type
    set(name "ubuntu-latest-Release")
    set(hash "7c855f601755455c6f2b653e7da08fc6957988de580baafef207d2de045a5c39")
  elseif(WIN32 AND CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    # MSVC debug and release runtimes do not mix, so the archive follows the build type
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
      set(name "windows-latest-Debug")
      set(hash "baaf4360d183006ca766d1343ee4c7961538dd4f6704ba2da8fb0f0ed11fd1ef")
    else()
      set(name "windows-latest-Release")
      set(hash "33d2bc3fb381d5dd3c765e0fa15947901687876c8b8ae9a53c0c0bebeb4a7802")
      get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
      if(multi_config)
        message(WARNING "wgpu-lab: prebuilt Dawn is the Release build. Debug configurations "
          "of a multi-config generator will not link against it; use a single-config "
          "generator (Ninja) or -DLAB_DAWN=source.")
      endif()
    endif()
  elseif(APPLE AND CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(name "macos-latest-Release")
    set(hash "fa453ded7d1f19e1d01885976f222cc00b5726053a1b026c24ff9d034ae1c0e1")
  elseif(APPLE)
    set(name "macos-15-intel-Release")
    set(hash "a13ec91e362127770324f7ec3ae07126e9e8eb13c70e4f4c18057910445b6e75")
  endif()
  set(${out_name} "${name}" PARENT_SCOPE)
  set(${out_hash} "${hash}" PARENT_SCOPE)
endfunction()

if(LAB_DAWN STREQUAL "prebuilt")
  lab_dawn_prebuilt_archive(lab_dawn_archive lab_dawn_hash)
  if(NOT lab_dawn_archive)
    message(STATUS "wgpu-lab: no prebuilt Dawn for ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}, building from source")
    set(LAB_DAWN "source")
  endif()
endif()

if(LAB_DAWN STREQUAL "prebuilt")
  message(STATUS "wgpu-lab: using prebuilt Dawn ${LAB_DAWN_TAG} (${lab_dawn_archive})")
  FetchContent_Declare(dawn_prebuilt
    URL "https://github.com/google/dawn/releases/download/${LAB_DAWN_TAG}/Dawn-${LAB_DAWN_SHA}-${lab_dawn_archive}.tar.gz"
    URL_HASH "SHA256=${lab_dawn_hash}"
    INACTIVITY_TIMEOUT 60 # fail instead of hanging forever if the download stalls
    DOWNLOAD_EXTRACT_TIMESTAMP ON)
  FetchContent_MakeAvailable(dawn_prebuilt)
  # the package config sits in lib/ or lib64/ depending on the archive, and CMake
  # does not search lib64/ on every distribution, so it is located explicitly
  file(GLOB lab_dawn_config "${dawn_prebuilt_SOURCE_DIR}/lib*/cmake/Dawn/DawnConfig.cmake")
  if(NOT lab_dawn_config)
    message(FATAL_ERROR "wgpu-lab: the Dawn archive has no DawnConfig.cmake, try -DLAB_DAWN=source")
  endif()
  list(GET lab_dawn_config 0 lab_dawn_config)
  cmake_path(GET lab_dawn_config PARENT_PATH lab_dawn_config_dir)
  find_package(Dawn CONFIG REQUIRED PATHS "${lab_dawn_config_dir}" NO_DEFAULT_PATH)

elseif(LAB_DAWN STREQUAL "source")
  message(STATUS "wgpu-lab: building Dawn ${LAB_DAWN_TAG} from source (this takes a while)")
  set(DAWN_FETCH_DEPENDENCIES ON)
  set(DAWN_ENABLE_INSTALL OFF)
  set(DAWN_BUILD_SAMPLES OFF)
  set(DAWN_BUILD_TESTS OFF)
  set(DAWN_BUILD_PROTOBUF OFF)
  set(DAWN_USE_GLFW OFF) # the lab brings its own GLFW and surface glue
  set(TINT_BUILD_TESTS OFF)
  set(TINT_BUILD_CMD_TOOLS OFF)
  set(TINT_BUILD_IR_BINARY OFF)
  # The source archive of the tag is used instead of a git clone: Dawn has thousands
  # of branches and tags, and even a shallow clone fetches the tip of each of them.
  FetchContent_Declare(dawn
    URL "https://github.com/google/dawn/archive/refs/tags/${LAB_DAWN_TAG}.tar.gz"
    INACTIVITY_TIMEOUT 60
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    EXCLUDE_FROM_ALL
    SYSTEM)
  FetchContent_MakeAvailable(dawn)
  if(NOT TARGET dawn::webgpu_dawn)
    add_library(dawn::webgpu_dawn ALIAS webgpu_dawn)
  endif()

elseif(LAB_DAWN STREQUAL "system")
  find_package(Dawn CONFIG REQUIRED)
  message(STATUS "wgpu-lab: using Dawn found at ${Dawn_DIR}")

else()
  message(FATAL_ERROR "wgpu-lab: LAB_DAWN must be prebuilt, source or system (got '${LAB_DAWN}')")
endif()
