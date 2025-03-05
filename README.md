# Low-Level Data Indexing

## Overview
This project implements a low-level data indexing system for vehicle fleet records in Brazil, using a B-tree index for optimized data retrieval. 
It is based on previous functionalities developed in earlier phases, which involve handling binary data files and performing operations such as searching, inserting, updating, and deleting records.

## Features
- **Data Storage**: Converts CSV data into binary format with fixed or variable-length records.
- **Record Selection**: Retrieves all or specific records based on criteria.
- **Indexing**: Creates and manages primary key indices (ID and RRN/byte offset) for efficient lookups.
- **Deletion & Updates**: Removes and modifies records while maintaining index integrity.
- **B-tree Indexing**: Supports fast lookups using a B-tree structure.
- **Batch Insertions**: Allows efficient record insertions while keeping indices up to date.

## File Structure
- `main.c` - Handles user input and executes corresponding commands.
- `commands.c` - Implements core functionality for data operations.
- `utils.c` - Helper functions for file management and input handling.
- `lib/` - Contains utility functions and predefined constants.

## Compilation & Execution
### Compilation
Use `gcc` to compile the project:
```sh
gcc -o vehicle_index main.c commands.c utils.c
```

### Running the Program
Execute the compiled binary and provide input via standard input:
```sh
./vehicle_index
```
Users will be prompted to enter a command, followed by necessary parameters.

## Supported Commands
| Command Code | Functionality |
|-------------|--------------|
| 1 | Create data table from CSV |
| 2 | Select all records |
| 3 | Select records by criteria |
| 4 | Select record by RRN |
| 5 | Create index file |
| 6 | Remove records |
| 7 | Insert records |
| 8 | Update records |
| 9 | Create B-tree index |
| 10 | Select by ID using B-tree |
| 11 | Insert into B-tree index |

## Error Handling
- **File Processing Errors**: If an operation fails, an error message is displayed: `Falha no processamento do arquivo.`
- **Record Not Found**: If a search does not return results: `Registro inexistente.`

## Authors
- Danielle Modesti (USP 12543544)
- Rafael Zimmer (USP 12542612)

## Contact
For any issues or questions, please contact the authors via email or through the university platform.

