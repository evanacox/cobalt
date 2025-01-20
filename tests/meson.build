tests_sources = [
    'support/filedep.cc',
    'bc/leb128.test.cc'
]

dependencies = [
    absl_base,
    absl_container,
    absl_hash,
    absl_strings,
    gtest,
    gtest_main
]

support = include_directories('support')

tests = executable('cobalt-tests', tests_sources,
                   dependencies : dependencies,
                   include_directories : [cobalt_includes, support],
                   link_with : cobalt)

test('cobalt tests', tests)