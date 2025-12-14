require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name         = "NitroDuckdb"
  s.version      = package["version"]
  s.summary      = package["description"]
  s.homepage     = package["homepage"]
  s.license      = package["license"]
  s.authors      = package["author"]

  s.platforms    = { :ios => '13.4', :visionos => 1.0 }
  s.source       = { :git => "https://github.com/walkthru-earth/react-native-duckdb.git", :tag => "#{s.version}" }

  s.source_files = [
    # Implementation (Swift)
    "ios/**/*.{swift}",
    # Autolinking/Registration (Objective-C++)
    "ios/**/*.{m,mm}",
    # Implementation (C++ objects)
    "cpp/**/*.{h,hpp,cpp}",
  ]

  # DuckDB pre-built framework
  s.vendored_frameworks = "ios/DuckDB.xcframework"

  # Preserve paths for DuckDB headers
  s.preserve_paths = [
    "cpp/duckdb.h",
    "ios/DuckDB.xcframework"
  ]

  load 'nitrogen/generated/ios/NitroDuckdb+autolinking.rb'
  add_nitrogen_files(s)

  s.dependency 'React-jsi'
  s.dependency 'React-callinvoker'
  install_modules_dependencies(s)
end
