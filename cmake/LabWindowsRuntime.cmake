# DLLs that Dawn's D3D12 backend loads at run time on Windows.
#
# Dawn compiles shaders through the DirectX Shader Compiler (dxcompiler.dll and
# dxil.dll) or the older d3dcompiler_47.dll, and it looks for them next to the
# executable only. Neither is part of the Dawn archive. The shader compiler is
# downloaded from its own releases, d3dcompiler_47.dll is taken from the Windows
# SDK if it is installed, and lab_copy_runtime_dlls() puts them next to a target.

include(FetchContent)

FetchContent_Declare(dxc
  URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.9.2609/dxc_2026_09_29.zip"
  URL_HASH "SHA256=ad31b1fc8443175d204f77a611fdb3ef2ec42759bdc2f1167368de24a4a7e7f1"
  INACTIVITY_TIMEOUT 60
  DOWNLOAD_EXTRACT_TIMESTAMP ON)
FetchContent_MakeAvailable(dxc)

# the archive holds bin/x64/, bin/x86/ and bin/arm64/
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|aarch64)$")
  set(lab_dxc_arch "arm64")
else()
  set(lab_dxc_arch "x64")
endif()

set(LAB_RUNTIME_DLLS "")
foreach(dll dxcompiler.dll dxil.dll)
  file(GLOB_RECURSE lab_dxc_candidates "${dxc_SOURCE_DIR}/*${dll}")
  list(FILTER lab_dxc_candidates INCLUDE REGEX "${lab_dxc_arch}")
  if(NOT lab_dxc_candidates)
    message(FATAL_ERROR "wgpu-lab: ${dll} for ${lab_dxc_arch} not found in the DirectX Shader Compiler archive")
  endif()
  list(GET lab_dxc_candidates 0 lab_dxc_dll)
  list(APPEND LAB_RUNTIME_DLLS "${lab_dxc_dll}")
endforeach()

find_file(LAB_D3DCOMPILER_DLL d3dcompiler_47.dll
  PATHS
    "$ENV{WindowsSdkDir}Redist/D3D/${lab_dxc_arch}"
    "$ENV{WindowsSdkDir}/Redist/D3D/${lab_dxc_arch}"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]/Redist/D3D/${lab_dxc_arch}"
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots;KitsRoot10]/Redist/D3D/${lab_dxc_arch}"
  NO_DEFAULT_PATH
  DOC "d3dcompiler_47.dll of the Windows SDK, loaded by Dawn next to the executable")
if(LAB_D3DCOMPILER_DLL)
  list(APPEND LAB_RUNTIME_DLLS "${LAB_D3DCOMPILER_DLL}")
else()
  message(STATUS "wgpu-lab: d3dcompiler_47.dll not found (no Windows SDK?), Dawn will use the DirectX Shader Compiler only")
endif()
message(STATUS "wgpu-lab: run time DLLs: ${LAB_RUNTIME_DLLS}")

# Copies the DLLs next to the executable `TARGET` after it is built.
# Programs that use the lab from another project call this for their executables.
function(lab_copy_runtime_dlls TARGET)
  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LAB_RUNTIME_DLLS} $<TARGET_FILE_DIR:${TARGET}>
    COMMENT "Copying the DLLs Dawn needs next to ${TARGET}"
    VERBATIM)
endfunction()
