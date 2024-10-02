project('jacktests','cpp',
        default_options : ['cpp_std=c++20'],  
        version : '0.0.1')

# Find Jack dependency
jack_dep = dependency('jack')

# Find Boost dependency
boost_dep = dependency('boost', modules : ['program_options'])

# Combine multiple dependencies
all_deps = [jack_dep, boost_dep]

# Define sources
sources = files('main.cpp', 'jack_client.cpp', 'waitkey.cpp', 'dsp_client.cpp')

# Generate executable
executable('dsp1', sources, dependencies : all_deps)
