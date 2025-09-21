# NKeilHelper

A Rust implementation of the CaKeilPlugin tool for Keil development environments. This tool helps with automatic file comment insertion and template processing for C/C++ source files.

## Features

- **File Comment Insertion**: Automatically adds header comments to C and header files
- **Template Processing**: Supports dynamic template variables like file name, date, year
- **Encoding Detection**: Automatically detects and preserves file encoding (UTF-8, UTF-16, etc.)
- **Cross-platform**: Written in Rust for better performance and cross-platform compatibility

## Installation

Make sure you have Rust installed on your system. Then clone and build the project:

```bash
git clone <repository-url>
cd NKeilHelper
cargo build --release
```

The executable will be available at `target/release/nkeil-helper` (or `nkeil-helper.exe` on Windows).

## Usage

### Basic Commands

```bash
# Show help information
nkeil-helper help

# Reset configuration
nkeil-helper reset

# Add file comment to a C/header file
nkeil-helper file-comment --file path/to/your/file.c
nkeil-helper file-comment --file path/to/your/file.h
```

### Template Files

The tool uses template files `File.c` and `File.h` which should be placed in the same directory as the executable or in the current working directory.

#### Template Variables

The following variables are supported in template files:

- `{FileName}`: The name of the target file
- `{Date}`: Current date in YYYY-MM-DD format
- `{Year}`: Current year
- `{ifndef}`: Auto-generated include guard for header files

#### Example Template (File.h)

```c
/*
 * File: {FileName}
 * Description: Header file template
 * Author: Your Name
 * Date: {Date}
 * Year: {Year}
 * 
 * Copyright (c) {Year} Your Company Name
 * All rights reserved.
 */

{ifndef}

/* Your header content goes here */
```

## Comparison with Original C# Version

| Feature | C# Version | Rust Version |
|---------|------------|--------------|
| Command Line Interface | Manual parsing | clap library |
| File Encoding Detection | Custom implementation | encoding_rs library |
| Error Handling | Exception-based | Result-based |
| Performance | .NET runtime | Native executable |
| Platform Support | Windows/.NET | Cross-platform |

## Development

### Project Structure

```
src/
├── main.rs          # Main application and CLI interface
├── encoding.rs      # File encoding detection
├── template.rs      # Template processing
└── file_ops.rs      # File operations
```

### Running Tests

```bash
cargo test
```

### Building for Release

```bash
cargo build --release
```

## Migration from C# Version

This Rust implementation maintains compatibility with the original C# version while providing:

1. **Better Error Handling**: Uses Rust's `Result` type for robust error handling
2. **Modern CLI**: Uses the `clap` library for better command-line interface
3. **Cross-platform Support**: No dependency on .NET runtime
4. **Better Performance**: Native compilation for better performance

### Command Mapping

| C# Version | Rust Version |
|------------|--------------|
| `-0` | `reset` |
| `-1` | `help` |
| `-2 <file>` | `file-comment --file <file>` |

## License

This project is licensed under the same terms as the original CaKeilPlugin.

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request