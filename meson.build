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
    '-T', meson.current_source_dir() / linker_script   # FIXED: use linker_script variable
]

# Find all source files
sources = run_command(
    'find', 'kernel', '-name', '*.cpp', '-o', '-name', '*.c', '-o', '-name', '*.asm',
    check: true
).stdout().strip().split('\n')

# Handle empty find result
if sources == ['']
    sources = []
endif

# Build the kernel
kernel = executable(
    'kernel.elf',
    sources,
    include_directories: incs,
    cpp_args: cpp_args,
    link_args: link_args,
    link_depends: linker_script
)

run_target('run',
    command: ['qemu-system-x86_64', '-kernel', kernel.full_path()],
    depends: kernel
)

run_target('img',
    command: ['toolchain/create-limine.sh', kernel.full_path(), 'gooseOS.img'],
    depends: kernel
)