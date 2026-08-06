Purpose

Provide concise, repository-specific guidance for Copilot sessions working on GooseOS (a small freestanding C++ OS).

Build, run and image commands

- Configure and build (recommended):
  - meson setup build && meson compile -C build
  - Equivalent: mkdir build && cd build && meson .. && ninja

- Build a single target (e.g., kernel ELF):
  - meson compile -C build kernel.elf
  - ninja -C build kernel.elf

- Create bootable image (uses toolchain/create-limine.sh and mkgoosefs):
  - meson compile -C build img
  - This runs: toolchain/create-limine.sh <kernel.elf> gooseOS.img <build/ramfs.img>
  - Note: create-limine.sh requires sudo and external tools (wget, dd, parted, mkfs.fat, losetup, mount, unzip).

- Run in QEMU (defined as a meson run_target):
  - meson compile -C build run
  - or: qemu-system-x86_64 -hdd gooseOS.img

Tests & linting

- There are no automated unit tests or lints configured in the repository. If adding tests, integrate them into meson.build and add run_target entries.

High-level architecture (big picture)

- kernel/: primary source tree for the OS kernel. Organized by arch under kernel/arch/<arch>/ (x86_64 present).
- Top-level build uses Meson + Ninja to cross-compile a freestanding C++ kernel (C++20) with many -f.../-m... flags set for kernel mode.
- Linker script per-architecture: kernel/arch/x86_64/linker.ld controls symbol layout and sections.
- Toolchain/: helper tools built for the host (mkgoosefs) and scripts (create-limine.sh) used to package the kernel into a bootable image.
- ramfs/: root filesystem contents included into the image by mkgoosefs; ramfs.img is created by a meson custom_target.
- build/: meson/ninja build directory (contains kernel.elf, gooseOS.img, kernel object files and meson-private metadata).
- Limine bootloader: limine.conf and toolchain/create-limine.sh are used to produce a UEFI bootable image.

Key conventions and repository-specific patterns

- Freestanding C++ environment:
  - Meson sets cpp_args with flags: -ffreestanding -nostdlib -fno-exceptions -fno-rtti -mno-red-zone, etc. Don’t add code that depends on the C++ standard library unless the build is adapted.

- Arch is chosen in meson.build (arch = 'x86_64') and arch-specific headers are included from kernel/arch/<arch>/include. To add/target another arch, adjust meson.build and add the arch tree and linker script.

- Source discovery: Meson uses a small run_command call that globs kernel/**/*.{cpp,c,asm} to generate the sources list. New files placed under kernel/ will be picked up automatically; if you add files elsewhere, update meson.build.

- Native tools vs target build: mkgoosefs is built as a native executable (native: true) and invoked during image creation. Keep host-only helpers in toolchain/ and avoid mixing host-only code into freestanding kernel sources.

- Image creation relies on system utilities and sudo (create-limine.sh). CI or automated runners must have those installed and allow loop device / mount operations or use a container/VM with required privileges.

Files to inspect when changing layout or build behavior

- meson.build (root): primary build configuration, cpp flags, run_targets and image creation targets.
- kernel/arch/<arch>/linker.ld: controls memory layout and symbols — changing this affects boot and linking.
- toolchain/create-limine.sh and toolchain/mkgoosefs.cpp: image packaging and ramfs builder.

Existing documentation to reuse

- readme.md: very brief project description and license. Important to preserve GPLv3 licensing when editing or redistributing.

Notes for Copilot sessions

- Prioritize changes to meson.build and linker script for build/layout tasks.
- For runtime/boot problems, inspect arch-specific boot/ and cpu/ directories under kernel/arch/x86_64.
- When adding features that require libc-like services, treat them as large changes: adjust cpp_args and build scripts; signal this in PR descriptions.

Summary

This file documents build/run/image commands, high-level repo architecture, and the key patterns Copilot should follow when suggesting edits. If additions are wanted (CI configurations, test harness, or code-style rules), say which area to cover next.

All contributions to GooseOS are licensed under GPLv3. AI-generated suggestions (including Copilot outputs) are permitted but must be reviewed by a human for correctness, safety, and style before merging.

Code styling

GooseOS is a freestanding C++20 project with no standard library, exceptions, or RTTI by default. When proposing code changes, keep suggestions minimal, explicit, and compliant with the freestanding environment:

- Prefer plain structs and free functions; use classes only when they clearly benefit the design and the reviewer verifies they follow freestanding constraints.
- Avoid types and APIs from the C++ standard library (std::string, std::vector, iostream) unless the build is intentionally changed to include them and the change is documented.
- Use the project Console APIs for output (Console::PrintString, Console::PrintChar, Console::INFO, Console::Log, Console::Error, Console::OK) rather than printf or std::cout.
- The console output uses a simple markup (e.g., C[fg,5] or C[r,]) implemented in kernel/arch/x86_64/console; follow examples there for formatting.

Types

Include kernel/include/types.hpp when you need fixed-width types and booleans. Common aliases defined there:

- bl — boolean type (use ktrue / kfalse values)
- u8, u16, u32, u64 — unsigned integers
- i8, i16, i32, i64 — signed integers
- size, ssize — sizes
- uintptr_t — pointer-size unsigned integer

Memory rules

- Do not perform heap allocation in interrupt handlers.
- Use the project's allocator APIs (see kernel/mem/alloc.hpp and implementations) for dynamic allocation. Avoid new/delete; they are not available in the freestanding environment.
- Check allocation results for null and free resources when no longer needed.

Build & toolchain guidance

- Meson is the supported build system. Changes to build behavior should update meson.build and maintain the cross/host separation: host tools belong in toolchain/ (built with native: true).
- Image creation and packaging are implemented in toolchain/ (mkgoosefs) and toolchain/create-limine.sh; those scripts rely on system utilities and may require sudo in local environments.
- Prefer adding meson targets (custom_target / run_target) rather than ad-hoc scripts for CI-friendly automation.

Architecture-specific code

- Place architecture-specific assembly or CPU/boot code under kernel/arch/<arch>/. Do not add architecture-specific asm or registers in common code outside the arch/ tree.

Debugging and errors

- Use the Console APIs for runtime messages and follow existing patterns in kernel/arch/x86_64/console/ for formatting and levels.
- Do not add busy infinite loops unless explicitly requested; prefer returning error codes or asserting with panic paths when appropriate.
- Address errors and warnings directly — avoid suppressing them in suggestions.

Where to look when changing core behavior

- meson.build (root): build flags, arch selection, run_targets, and custom targets.
- kernel/arch/<arch>/: boot, startup, and low-level code for each supported architecture.
- kernel/include/ and kernel/mem/: core headers and allocator APIs.
- toolchain/: host helpers and image packaging code.

If you'd like, commit this cleaned version to the repository and/or add CI that runs meson/ninja and image creation in a privileged runner. Let me know which action to take.