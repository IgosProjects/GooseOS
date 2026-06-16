# Tell Meson how to handle NASM files
nasm = find_program('nasm', required: true)

# Custom rule for .asm -> .o
asm_generator = generator(nasm,
    output: '@BASENAME@.o',
    arguments: ['-f', 'elf64', '@INPUT@', '-o', '@OUTPUT@']
)

# Process assembly files
asm_files = [
    'boot.asm',
    # add other .asm files here
]

foreach asm : asm_files
    asm_obj = asm_generator.process(asm)
    # Add to sources somehow - this is tricky
endforeach