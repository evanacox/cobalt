project('cobalt', 'cpp',
        version : '0.0.1',
        default_options : ['warning_level=3', 'werror=true', 'cpp_std=c++20'])

if get_option('buildtype') == 'debug'
    add_project_arguments('-DCOBALT_DEBUG', '-g3',  language : 'cpp')
endif

cobalt_includes = include_directories('include')

absl_dep = subproject('abseil-cpp', default_options: ['warning_level=0', 'werror=false'])
benchmark_dep = subproject('google-benchmark', default_options: ['warning_level=0', 'werror=false'])
gtest_dep = subproject('gtest', default_options: ['warning_level=0', 'werror=false'])

# pull deps out of the subprojects
absl_base = absl_dep.get_variable('absl_base_dep').as_system()
absl_container = absl_dep.get_variable('absl_container_dep').as_system()
absl_hash = absl_dep.get_variable('absl_hash_dep').as_system()
absl_strings = absl_dep.get_variable('absl_strings_dep').as_system()
gtest = gtest_dep.get_variable('gtest_dep').as_system()
gtest_main = gtest_dep.get_variable('gtest_main_dep').as_system()
benchmark = benchmark_dep.get_variable('google_benchmark_dep').as_system()

subdir('src')
subdir('tests')
subdir('tools')
subdir('benchmarks')