project('jpeg-test', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3',
                     'cpp_std=c++14'])

jpeg_dep = dependency('libjpeg')
cv4_dep = dependency('opencv4')

exe = executable('jpeg_test', 'jpeg_test.cpp',
  dependencies: [jpeg_dep, cv4_dep],
  install : true)

test('basic', exe)
