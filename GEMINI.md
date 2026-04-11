# ireporter: Gemini AI Context File

## Project Overview
This repository contains `ireporter`, a telemetry reporter tool. It is built using a core C++ implementation with supplementary Python logic and relies on CMake as its build system.

## Architectural Guidelines
- **Core Logic:** The high-performance core is written in C++ (e.g., `ireporter.cpp`). Performance and memory safety are critical here.
- **Scripting & Wrappers:** Python is used for supplementary scripting and potential language bindings (`ireporter.py`). Ensure Python code is idiomatic and handles errors gracefully.
- **Build System:** All build configurations should be maintained within `CMakeLists.txt`. When adding new C++ source files, ensure they are properly linked in CMake.

## Coding Standards

### C++ Conventions
- Use modern C++ features where applicable.
- Ensure strict memory management. Avoid raw pointers where smart pointers (`std::unique_ptr`, `std::shared_ptr`) can be used.
- All new public functions must have descriptive comments/Doxygen documentation.
- Maintain consistent indentation and brace styling.

### Python Conventions
- Follow PEP-8 style guidelines.
- Use type hints wherever possible to improve readability and maintainability.
- Keep dependencies minimal to ensure the reporter is lightweight.

## Testing and Validation
- When modifying C++ core logic, ensure you update or provide corresponding unit tests.
- When modifying Python scripts, ensure they are executed to verify the expected behavior and that telemetry is correctly reported.

## Common Workflows
- **Building (Clang/Ninja):** Run `cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=RelWithDebInfo` followed by `ninja -C build` to compile the C++ binaries.
- **IIS Deployment:** Copy `ireporter.py`, `web.config`, and `templates/` to `C:\inetpub\wwwroot\ireporter`. Ensure the folder is configured as an IIS Application.
- **Dashboard Access:** Visit `http://localhost/ireporter/` to monitor and control the telemetry receiver.
- **Client Execution:** Run the generated binary from the `build` directory to test telemetry reporting to the local IIS server.
