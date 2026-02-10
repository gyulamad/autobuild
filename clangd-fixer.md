# clangd-fixer

A utility program that generates the `.clangd` configuration file for the clangd language server.

## Purpose

The `clangd-fixer` tool processes a template file (`.clangd.tpl`) and generates a fully configured `.clangd` file with the correct project root path substituted in all template placeholders.

## Usage

```bash
./builder autobuild/clangd-fixer.cpp --run
```

## How it works

1. Reads the template file `.clangd.tpl`
2. Replaces the `{{PROJECT_ROOT}}` placeholder with the current working directory
3. Writes the result to `.clangd`

## Requirements

- The template file `.clangd.tpl` must exist in the same directory
- The template must contain the `{{PROJECT_ROOT}}` placeholder

## Exit codes

- `0` - Success (configuration file generated)
- `1` - Failure (file write operation failed)

## Template format

The `.clangd.tpl` file uses C++ style template syntax with double curly braces:

```yaml
CompileFlags:
  Add:
    - -std=c++20
    - -I{{PROJECT_ROOT}}/cpptools/misc
    - -I{{PROJECT_ROOT}}/cpptools/ai
```

After processing, `{{PROJECT_ROOT}}` is replaced with the absolute path to the project root.

## Example

**Input (`.clangd.tpl`):**
```yaml
CompileFlags:
  Add:
    - -I{{PROJECT_ROOT}}/include
```

**Output (`.clangd`) - if current directory is `/home/user/project`:**
```yaml
CompileFlags:
  Add:
    - -I/home/user/project/include
```

## Related files

- [`autobuild/clangd-fixer.cpp`](autobuild/clangd-fixer.cpp) - The source code
- `.clangd.tpl` - The template file
- `.clangd` - The generated configuration file