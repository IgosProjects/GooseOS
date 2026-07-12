project('gooseOS', 'cpp', 'nasm',
    default_options: [
        'cpp_std=c++20',
        'optimization=2',
        'warning_level=3'
    ]
)

arch = 'x86_64'
arch_include = 'kernel/arch' / arch / 'include'

# Compiler flags for bare metal C++
cpp_args = [
    '-ffreestanding',
    '-nostdlib',
    '-mcmodel=kernel',    
    '-fno-builtin',
    '-fno-stack-protector',
    '-mgeneral-regs-only',
    '-mno-red-zone',
    '-fno-exceptions',
    '-fno-rtti',
    '-fno-pic',
    '-fno-pie'
]

incs = [
    include_directories('kernel/include'),
    include_directories('kernel/arch/' + arch + '/include')
]

linker_script = 'kernel/arch' / arch / 'linker.ld'

link_args = [
    '-nostdlib',
    '-static',
    '-fno-pie',
    '-T', meson.current_source_dir() / linker_script
]

fs = import('fs')
sources = []

# Call a simple python command to find all files recursively
find_src = run_command('python3', '-c', 
    'import glob; import sys; [print(f) for ext in ["/**/*.cpp", "/**/*.c", "/**/*.asm"] for f in glob.glob("kernel" + ext, recursive=True)]',
    check: true
)

sources = find_src.stdout().strip().split()

kernel = executable(
    'kernel.elf',
    sources: sources,
    include_directories: incs,
    cpp_args: cpp_args,
    link_args: link_args,
    link_depends: linker_script
)

# Limine configuration file
limine_conf = configure_file(
  input: 'limine.conf',
  output: 'limine.conf',
  copy: true
)

run_target('run',
    command: ['qemu-system-x86_64', '-hdd', 'gooseOS.img'],
    depends: [kernel]
)

run_target('img',
    command: ['toolchain/create-limine.sh', kernel.full_path(), 'gooseOS.img', join_paths(meson.current_build_dir(), 'extra_files'), limine_conf],
    depends: [kernel]
)