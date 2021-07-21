# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Modifies the build files of the Firestore C++ SDK to run tests with ASAN.

This script only modifies the files for Android. In the future, it is planned to
also support desktop and iOS build targets. Also, this script only supports
Address Sanitizer. In the future, it is planned to support other sanitizers,
such as Memory Sanitizer, Thread Sanitizer, and Undefined Behavior Sanitizer.
"""

import dataclasses
import os
import pathlib
import re
import shutil
import sys
from typing import Iterable, Optional, Sequence, Tuple

from absl import app
from absl import flags
from absl import logging

_FLAG_DIR = flags.DEFINE_string(
  "repo_dir",
  None,
  "The root directory of the checkout of the Firebase C++ SDK git repository. "
  "The contents of this directory will be modified to enable the specified "
  "sanitizer. (default: the current directory)"
)

_FLAG_AGP_VERSION = flags.DEFINE_string(
  "agp_version",
  "4.2.2",
  "The version of the Android Gradle Plugin (com.android.tools.build:gradle) "
  "to set in build.gradle files."
)

_FLAG_GMS_VERSION = flags.DEFINE_string(
  "gms_version",
  "4.3.8",
  "The version of the GMS Gradle Plugin (com.google.gms:google-services) "
  "to set in build.gradle files."
)

_FLAG_GRADLE_VERSION = flags.DEFINE_string(
  "gradle_version",
  "7.1.1",
  "The version of Gradle to use."
)

_FLAG_COMPILE_SDK_VERSION = flags.DEFINE_string(
  "compile_sdk_version",
  "30",
  "The value to set for `compileSdkVersion` in build.gradle files."
)

_FLAG_MIN_SDK_VERSION = flags.DEFINE_string(
  "min_sdk_version",
  "27",
  "The value to set for `minSdkVersion` in build.gradle files. "
  "Note that Address Sanitizer is only supported in Android API levels 27 "
  "(Android O MR 1) and later."
)

_FLAG_TARGET_SDK_VERSION = flags.DEFINE_string(
  "target_sdk_version",
  None,
  "The value to set for `targetSdkVersion` in build.gradle files. "
  f"(default: the value of --{_FLAG_MIN_SDK_VERSION.name})"
)

_FLAG_ANDROID_SDK_DIR = flags.DEFINE_string(
  "android_sdk_dir",
  None,
  "The path of the Android SDK installation directory. "
  f"(default: the value of the ANDROID_HOME environment variable, or "
  "ANDROID_SDK_ROOT if ANDROID_HOME is not set."
)

_FLAG_NDK_VERSION = flags.DEFINE_string(
  "ndk_version",
  "22.1.7171670",
  "The version of the Android NDK to use."
)


@dataclasses.dataclass(frozen=True)
class Settings:
  repo_dir: pathlib.Path
  android_sdk_dir: pathlib.Path
  agp_version: str
  gms_version: str
  gradle_version: str
  compile_sdk_version: str
  min_sdk_version: str
  target_sdk_version: str
  ndk_version: str


def main(argv: Sequence[str]) -> Optional[int]:
  if len(argv) > 1:
    raise app.UsageError(f"unexpected argument: {argv[1]}")
  del argv

  if _FLAG_DIR.value is not None:
    repo_dir = pathlib.Path(_FLAG_DIR.value)
  else:
    repo_dir = pathlib.Path.cwd()

  if _FLAG_TARGET_SDK_VERSION.value is not None:
    target_sdk_version = _FLAG_TARGET_SDK_VERSION.value
  else:
    target_sdk_version = _FLAG_MIN_SDK_VERSION.value

  if _FLAG_ANDROID_SDK_DIR.value is not None:
    android_sdk_dir = pathlib.Path(_FLAG_ANDROID_SDK_DIR.value)
  else:
    android_sdk_dir_candidates = [
      os.environ.get("ANDROID_HOME"),
      os.environ.get("ANDROID_SDK_ROOT"),
    ]
    for android_sdk_dir_candidate in android_sdk_dir_candidates:
      if android_sdk_dir_candidate:
        android_sdk_dir = pathlib.Path(android_sdk_dir_candidate)
        break
    else:
      raise app.UsageError(
        f"--{_FLAG_ANDROID_SDK_DIR.name} must be specified or one or both of the "
        "ANDROID_HOME or ANDROID_SDK_ROOT environment variables must be set.")

  settings = Settings(
    repo_dir=repo_dir,
    android_sdk_dir=android_sdk_dir,
    agp_version=_FLAG_AGP_VERSION.value,
    gms_version=_FLAG_GMS_VERSION.value,
    gradle_version=_FLAG_GRADLE_VERSION.value,
    compile_sdk_version=_FLAG_COMPILE_SDK_VERSION.value,
    min_sdk_version=_FLAG_MIN_SDK_VERSION.value,
    target_sdk_version=target_sdk_version,
    ndk_version=_FLAG_NDK_VERSION.value,
  )

  sanitizer_setup = SanitizerSetup(settings)
  try:
    sanitizer_setup.run()
  except sanitizer_setup.Error as e:
    print(f"ERROR: {e}", file=sys.stderr)
    return 1


class SanitizerSetup:

  def __init__(self, settings: Settings) -> None:
    self.settings = settings

  def run(self) -> None:
    self.check_repo_dir_name()
    self.tweak_files()
    self.copy_ndk_files()
    self.create_gradle_properties()

  def check_repo_dir_name(self) -> None:
    if self.settings.repo_dir.name != "firebase_cpp_sdk":
      raise self.Error(
        "The name of the directory into which the GitHub repository is "
        "checked out must be 'firebase_cpp_sdk', not "
        f"'{self.settings.repo_dir.name}' (full path: "
        f"{self.settings.repo_dir}). This it to support newer Gradle "
        "versions that don't honor setting rootProject.name='firebase_cpp_sdk' "
        "in settings.gradle."
      )

  def tweak_files(self) -> None:
    for path in self.settings.repo_dir.glob("**/*"):
      if path.name == "build.gradle":
        self.tweak_build_dot_gradle(path)
      elif path.name == "AndroidManifest.xml":
        self.tweak_android_manifest(path)
      elif path.name == "CMakeLists.txt":
        self.tweak_cmakelists_txt(path)
      elif path.name == "gradle-wrapper.properties":
        self.tweak_gradle_wrapper_properties(path)

  def tweak_build_dot_gradle(self, path: pathlib.Path) -> None:
    logging.info("Tweaking %s", path)
    tweaked_lines = "".join(self.tweaked_build_dot_gradle_lines(path))
    path.write_text(tweaked_lines, encoding="utf8")

  def tweaked_build_dot_gradle_lines(self, path: pathlib.Path) -> Iterable[str]:
    agp_pattern = "(?<=" + re.escape("com.android.tools.build:gradle:") + r")(\d+\.\d+\.\d+)"
    gms_pattern = "(?<=" + re.escape("com.google.gms:google-services:") + r")(\d+\.\d+\.\d+)"
    compile_sdk_pattern = r"(?<=compileSdkVersion )(\d+)"
    min_sdk_pattern = r"(?<=minSdkVersion )(\d+)"
    target_sdk_pattern = r"(?<=targetSdkVersion )(\d+)"
    external_native_build_found = False
    cmake_arguments_line_found = False

    with path.open("rt", encoding="utf8") as f:
      for line in f:
        # Remove specification of `buildToolsVersion` in favor of the default.
        if "buildToolsVersion" in line:
          continue

        # Specify to use shared libc, which is required by Address Sanitizer.
        if line.strip().startswith("arguments "):
          cmake_arguments_line_found = True
          extra_arguments = "'-DANDROID_STL=c++_shared', '-DANDROID_ARM_MODE=arm'"
          if line.rstrip().endswith(","):
            yield line + extra_arguments + ",\n"
          else:
            yield line.rstrip() + ", " + extra_arguments + "\n"
          continue

        # Set the NDK version
        if line.strip() == "android {":
          yield line
          yield f"  ndkVersion '{self.settings.ndk_version}'\n"
          continue

        # Remember if this file appears to build native targets.
        if line.strip().startswith("externalNativeBuild.cmake"):
          external_native_build_found = True

        # Replace version numbers
        line = re.sub(agp_pattern, self.settings.agp_version, line)
        line = re.sub(gms_pattern, self.settings.gms_version, line)
        line = re.sub(compile_sdk_pattern, self.settings.compile_sdk_version, line)
        line = re.sub(min_sdk_pattern, self.settings.min_sdk_version, line)
        line = re.sub(target_sdk_pattern, self.settings.target_sdk_version, line)

        yield line

    if external_native_build_found and not cmake_arguments_line_found:
      raise self.Error(
        f"No line found that begins with 'arguments' was found in {path}"
      )

  def tweak_android_manifest(self, path: pathlib.Path) -> None:
    logging.info("Tweaking %s", path)
    tweaked_lines = "".join(self.tweaked_android_manifest_lines(path))
    path.write_text(tweaked_lines, encoding="utf8")

  def tweaked_android_manifest_lines(self, path: pathlib.Path) -> Iterable[str]:
    app_element_pattern = "<application"
    with path.open("rt", encoding="utf8") as f:
      for line in f:
        index = line.find(app_element_pattern)
        if index >= 0:
          insert_index = index + len(app_element_pattern)
          # Set extractNativeLibs=true, as required by Address Sanitizer.
          yield (
            line[:insert_index] +
            " android:extractNativeLibs=\"true\"" +
            line[insert_index:]
          )
        else:
          yield line

  def tweak_cmakelists_txt(self, path: pathlib.Path) -> None:
    logging.info("Tweaking %s", path)
    tweaked_lines = "".join(self.tweaked_cmakelists_txt_lines(path))
    path.write_text(tweaked_lines, encoding="utf8")

  def tweaked_cmakelists_txt_lines(self, path: pathlib.Path) -> Iterable[str]:
    add_executable_token = "add_executable("
    executable_target_names = []

    with path.open("rt", encoding="utf8") as f:
      for line in f:
        if line.strip().startswith("cmake_minimum_required("):
          yield line
          yield (
            'set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} '
            '-fsanitize=address -fno-omit-frame-pointer")\n'
          )
          yield (
            'set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} '
            '-fsanitize=address -fno-omit-frame-pointer")\n'
          )
          yield (
            'set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} '
            '-fsanitize=address")\n'
          )
          yield (
            'set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} '
            '-fsanitize=address")\n'
          )
        elif line.strip().startswith(add_executable_token):
          yield line
          target_name_start = (line.index(add_executable_token) +
            len(add_executable_token))
          target_name_end = line.find(" ", target_name_start)
          if target_name_end >= 0:
            target_name = line[target_name_start:target_name_end].strip()
          else:
            target_name = line[target_name_start:].strip()
          executable_target_names.append(target_name)
        else:
          yield line

    for executable_target_name in executable_target_names:
      yield (
        f"target_compile_options({executable_target_name} "
        "PUBLIC -fsanitize=address -fno-omit-frame-pointer)\n"
      )
      yield (
        f"set_target_properties({executable_target_name} "
        "PROPERTIES LINK_FLAGS -fsanitize=address)\n"
      )

  def tweak_gradle_wrapper_properties(self, path: pathlib.Path) -> None:
    logging.info("Tweaking %s", path)
    tweaked_lines = "".join(self.tweaked_gradle_wrapper_properties_lines(path))
    path.write_text(tweaked_lines, encoding="utf8")

  def tweaked_gradle_wrapper_properties_lines(self, path: pathlib.Path) \
      -> Iterable[str]:
    with path.open("rt", encoding="utf8") as f:
      for line in f:
        if line.strip().startswith("distributionUrl"):
          yield (
            "distributionUrl=https\\://services.gradle.org/distributions/"
            f"gradle-{self.settings.gradle_version}-all.zip"
          )
        else:
          yield line

  def copy_ndk_files(self) -> None:
    for build_gradle_path in self.settings.repo_dir.glob("**/build.gradle"):
      if self.is_android_app_build_dot_gradle(build_gradle_path):
        self.copy_ndk_files_to_android_app(build_gradle_path.parent)

  def is_android_app_build_dot_gradle(self, path: pathlib.Path) -> bool:
    with path.open("rt", encoding="utf8") as f:
      for line in f:
        if line.strip() == "apply plugin: 'com.android.application'":
          return True
      else:
        return False

  def copy_ndk_files_to_android_app(self, path: pathlib.Path) -> None:
    for (arch_dir_name, so_path) in self.ndk_so_files():
      dest_dir = path / "libs" / arch_dir_name
      logging.info("Copying %s to %s", so_path, dest_dir)
      dest_dir.mkdir(parents=True, exist_ok=True)
      dest_file_str = shutil.copy(so_path, dest_dir)
      dest_file = pathlib.Path(dest_file_str)
      dest_file.chmod(0o755)

    for (arch_dir_name, wrap_sh_path) in self.ndk_wrap_sh_files():
      dest_file = (
        path / "src" / "main" / "resources" / "lib" / arch_dir_name / "wrap.sh"
      )
      logging.info("Copying %s to %s", wrap_sh_path, dest_file)
      dest_file.parent.mkdir(parents=True, exist_ok=True)
      shutil.copy(wrap_sh_path, dest_file)
      dest_file.chmod(0o755)
      self.tweak_ndk_wrap_sh(dest_file)

  def ndk_so_files(self) -> Iterable[Tuple[str, pathlib.Path]]:
    ndk_dir = self.settings.android_sdk_dir / "ndk" / self.settings.ndk_version
    prebuilt_dir = ndk_dir / "toolchains" / "llvm" / "prebuilt"

    ndk_libs = {
      "arm64-v8a": "libclang_rt.asan-aarch64-android.so",
      "armeabi-v7a": "libclang_rt.asan-arm-android.so",
      "x86": "libclang_rt.asan-i686-android.so",
      "x86_64": "libclang_rt.asan-x86_64-android.so",
    }

    for (arch_dir_name, lib_filename) in tuple(ndk_libs.items()):
      paths = sorted(prebuilt_dir.glob("**/" + lib_filename))
      if len(paths) == 0:
        raise self.Error(f"{lib_filename} not found in {prebuilt_dir}")
      elif len(paths) > 1:
        raise self.Error(
          f"Found {len(paths)} {lib_filename} files in {prebuilt_dir}, "
          f"but expected exactly 1: {', '.join(paths)}")
      else:
        yield (arch_dir_name, paths[0])

  def ndk_wrap_sh_files(self) -> Iterable[Tuple[str, pathlib.Path]]:
    ndk_dir = self.settings.android_sdk_dir / "ndk" / self.settings.ndk_version
    wrap_sh_dir = ndk_dir / "wrap.sh"

    wrap_sh_files = tuple(path for path in wrap_sh_dir.iterdir() if path.is_file())
    if len(wrap_sh_files) == 0:
      raise self.Error(f"No wrap.sh files were found in f{wrap_sh_dir}")

    arch_dir_names = [
      "arm64-v8a",
      "armeabi-v7a",
      "x86",
      "x86_64",
    ]

    # Use the same `wrap.sh` script for all architectures if there is only
    # one script, which, for example, is the case in NDK versions 21.4.7075529
    # and 22.1.7171670.
    if len(wrap_sh_files) == 1:
      for arch_dir_name in arch_dir_names:
        yield (arch_dir_name, wrap_sh_files[0])
    # Use the architecture-specific `wrap.sh` script if there is one provided
    # for each architecture, which, for example, is the case in NDK version
    # 17.2.4988734.
    else:
      for arch_dir_name in arch_dir_names:
        matching_wrap_sh_files = tuple(
          path for path in wrap_sh_files
          if path.name == f"asan.{arch_dir_name}.sh"
        )
        if len(matching_wrap_sh_files) == 0:
          raise self.Error(
            f"No wrap.sh script for architecture {arch_dir_name} "
            f"found in {wrap_sh_dir}"
          )
        elif len(matching_wrap_sh_files) > 1:
          raise self.Error(
            f"{len(matching_wrap_sh_files)} wrap.sh scripts for architecture "
            f"{arch_dir_name} found in {wrap_sh_dir}, but expected exactly 1: "
            f"{', '.join(path.name for path in matching_wrap_sh_files)}"
          )
        yield (arch_dir_name, matching_wrap_sh_files[0])

  def tweak_ndk_wrap_sh(self, path: pathlib.Path) -> None:
    logging.info("Tweaking %s", path)
    tweaked_lines = "".join(self.tweaked_ndk_wrap_sh_lines(path))
    path.write_text(tweaked_lines, encoding="utf8")

  def tweaked_ndk_wrap_sh_lines(self, path: pathlib.Path) -> Iterable[str]:
    asan_options_found = False
    with path.open("rt", encoding="utf8") as f:
      for line in f:
        if line.strip().startswith("export ASAN_OPTIONS="):
          asan_options_found = True
          # Turn off color to avoid distracting ANSI escape sequences in logcat.
          yield line.rstrip() + ",color=never\n"
        else:
          yield line

    if not asan_options_found:
      raise self.Error("ASAN_OPTIONS not found in %s", path)

  def create_gradle_properties(self) -> None:
    self.update_gradle_properties(self.settings.repo_dir / "gradle.properties")
    for build_gradle_path in self.settings.repo_dir.glob("**/build.gradle"):
      if self.is_gradle_root_project(build_gradle_path):
        self.update_gradle_properties(build_gradle_path.parent / "gradle.properties")

  def is_gradle_root_project(self, path: pathlib.Path) -> bool:
    with path.open("rt", encoding="utf8") as f:
      for line in f:
        if line.strip() == "buildscript {":
          return True
      else:
        return False

  def update_gradle_properties(self, path: pathlib.Path) -> bool:
    logging.info("Updating %s", path)
    updated_lines = "".join(self.updated_gradle_properties_lines(path))
    path.write_text(updated_lines, encoding="utf8")

  def updated_gradle_properties_lines(self, path: pathlib.Path) -> Iterable[str]:
    if path.is_file():
      with path.open("rt", encoding="utf8") as f:
        yield from f
    yield "android.useAndroidX=true\n"

  class Error(Exception):
    """
    Exception raised if an error occurs in this class.
    """

if __name__ == "__main__":
  app.run(main)
