{ stdenv, pkgs }:
{ xcodeBaseDir ? "/Applications/Xcode.app" }:

if stdenv.isDarwin then
  stdenv.mkDerivation {
    name = "xcode-wrapper";
    buildCommand = ''
      mkdir -p $out/bin
      cd $out/bin
      ln -s /usr/bin/xcode-select
      ln -s /usr/bin/security
      ln -s /usr/bin/codesign
      ln -s /usr/bin/xcrun
      ln -s /usr/bin/plutil
      ln -s /usr/bin/clang
      ln -s /usr/bin/clang++
      ln -s /usr/bin/lipo
      ln -s /usr/bin/file
      ln -s /usr/bin/swift
      ln -s /usr/bin/rev
      ln -s "${xcodeBaseDir}/Contents/Developer/usr/bin/xcodebuild"
      ln -s "${xcodeBaseDir}/Contents/Developer/Applications/Simulator.app/Contents/MacOS/Simulator"
      cd ..
      ln -s "${xcodeBaseDir}/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs"

      # Print the host xcode version
      echo "Found host xcode version: $($out/bin/xcodebuild -version)"
    '';
  }
else
  stdenv.mkDerivation { name = "empty-xcode-wrapper"; }
