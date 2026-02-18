# ArxConverter

A command-line tool for reading, parsing, and exporting 3D models from the `.ftl` format.

---

## Features

- Reading and decompressing `.ftl` files (ArxExplode format)
- Parsing binary data: vertices, polygons, vertex groups, texture paths, action points, collision spheres, progressive mesh, cloth simulation
- Exporting data to multiple formats:
  - **JSON**
  - **XML**
  - **OBJ + MTL**
  - **glTF** (beta)

---

## Requirements

- Compiler with **C++23** support
- Recommended: MSVC 19.35+ or Clang 17+
- Linux: untested

## Usage

```
ArxConverter.exe <file.ftl> [output_directory]
```

| Argument             | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| `<file.ftl>`         | Path to the input `.ftl` file (required)                                   |
| `[output_directory]` | Directory to save results (optional). Defaults to the input file's folder  |

### Examples

```bash
# Export next to the source file
ArxConverter.exe models/goblin.ftl

# Export to a specified directory
ArxConverter.exe models/goblin.ftl C:\Export
```

In both cases, a subdirectory named after the input file's stem (e.g. `goblin/`) will be created inside the output directory, containing the exported files.

---

## Project Structure

| File                    | Description                                                                 |
|-------------------------|-----------------------------------------------------------------------------|
| `ArxConverter.ixx/.cpp` | Orchestrator: parses arguments, creates the output directory, coordinates components |
| `ArxFile.ixx/.cpp`      | Reads the `.ftl` file from disk and decompresses it via `ArxExplode`       |
| `ArxParser.ixx/.cpp`    | Parses binary data from the decompressed file into structured types         |
| `ArxExporter.ixx/.cpp`  | Exports parsed data to JSON, XML, OBJ/MTL, and glTF                        |

---

## FTL Format

An FTL file contains:

- **Primary header** — `FTL` signature and format version
- **Secondary header** — offsets to sections: 3D data, collision spheres, progressive mesh, cloth data
- **3D data** — vertices, polygons, texture paths (256 bytes per path), vertex groups, action points, vertex selections
- **Collision Spheres** — spheres used for collision physics
- **Progressive Mesh** — data for the LOD system
- **Cloth Data** — vertices and springs for cloth simulation

For a detailed description of all structures, see **ArxHeaders.ixx**.

<img width="591" height="591" alt="изображение" src="https://github.com/user-attachments/assets/9c37d048-6e48-455b-b940-86b4c9bcb630" />

<img width="519" height="367" alt="изображение" src="https://github.com/user-attachments/assets/97d3fe87-2b1c-418a-a475-09575db14c3f" />
