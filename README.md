# SyncLite

A lightweight file synchronization utility written in C, inspired by the core idea behind `rsync`.

The project demonstrates how a synchronization system can avoid retransmitting data that already exists in the destination file by using fixed-size blocks, checksums, rolling checksum-based searching, and COPY/INSERT operations.

---

## Features

- Fixed-size block processing
- Simple additive checksum
- Rolling checksum for efficient searching
- Byte-level verification after checksum matches
- Checksum collision handling
- COPY and INSERT synchronization operations
- Synchronization statistics
- Temporary-file based synchronization
- Safe destination replacement using `rename()`
- Robust handling of partial reads and writes
- CLI-based interface
- Edge-case testing
- End-to-end integration testing
- Modular C project structure
- Makefile-based build support

---

## How It Works

The basic idea is to compare a source file with an existing destination file.

Instead of simply rewriting the entire destination, `mini-rsync` attempts to reuse data that is already present in the destination.

The source file is divided into fixed-size blocks.

For each source block:

1. Calculate its checksum.
2. Search the destination for a matching checksum using a rolling checksum.
3. Verify the actual bytes if the checksum matches.
4. If the block exists in the destination, create a `COPY` operation.
5. If the block does not exist, create an `INSERT` operation.
6. Apply all operations to a temporary output file.
7. Replace the destination using `rename()`.

### Synchronization Flow

```text
                     Source File
                          |
                          v
                  Split into blocks
                          |
                          v
                  Calculate checksum
                          |
                          v
               Search destination file
                          |
                 +--------+--------+
                 |                 |
              Match            No Match
                 |                 |
                 v                 v
               COPY             INSERT
                 |                 |
                 +--------+--------+
                          |
                          v
                  Temporary Output
                          |
                          v
                       rename()
                          |
                          v
                 Updated Destination
```

---

## Project Structure

```text
SyncLite/
│
├── mini-rsync/
│   │
│   ├── include/
│   │   ├── block.h
│   │   ├── file.h
│   │   ├── rolling_checksum.h
│   │   └── sync.h
│   │
│   ├── src/
│   │   ├── block.c
│   │   ├── file.c
│   │   ├── main.c
│   │   ├── rolling_checksum.c
│   │   └── sync.c
│   │
│   ├── tests/
│   │   └── test_sync.c
│   │
│   └── Makefile
│
├── .gitignore
├── LICENSE
└── README.md
```

### Directory Description

| Directory/File | Purpose |
|---|---|
| `include/` | Header files and public declarations |
| `src/` | Main C implementation |
| `tests/` | Test programs and test data |
| `block.c` | Block creation and checksum logic |
| `file.c` | File opening, reading, writing and closing |
| `rolling_checksum.c` | Rolling checksum and block searching |
| `sync.c` | Synchronization operations |
| `main.c` | CLI entry point |
| `Makefile` | Build automation |
| `.gitignore` | Prevents build artifacts from being committed |
| `LICENSE` | Project license |
| `README.md` | Project documentation |

---

## Requirements

### Required

- GCC
- POSIX-compatible environment

### Optional

- GNU Make

The project has been developed and tested using GCC inside WSL on Windows.

---

## Building the Project

### Using Make

From the `mini-rsync` directory:

```bash
make
```

This builds:

```text
mini-rsync
```

You can then run:

```bash
./mini-rsync <source> <destination>
```

### Manual Compilation

If Make is not available, compile manually:

```bash
gcc -Wall -Wextra -Wpedantic -Iinclude \
src/main.c \
src/file.c \
src/block.c \
src/rolling_checksum.c \
src/sync.c \
-o mini-rsync
```

The compiler flags enable additional warnings to help catch potential problems during development.

---

## Usage

The basic syntax is:

```bash
./mini-rsync <source> <destination>
```

For example:

```bash
./mini-rsync source.txt destination.txt
```

The program attempts to synchronize the destination file so that it contains the same data as the source file.

---

## Example

Create a source file:

```bash
printf "ABCDEF" > source.txt
```

Create a destination containing some of the same data:

```bash
printf "XXABCDEFYY" > destination.txt
```

Run:

```bash
./mini-rsync source.txt destination.txt
```

Example output:

```text
Generated 2 synchronization operations

Synchronization Statistics
---------------------------
Source size       : 6 bytes
Destination size  : 10 bytes
Operations        : 2
Bytes copied      : 6
Bytes inserted    : 0
Bytes transferred : 0

Synchronization completed
```

Verify the result:

```bash
cmp source.txt destination.txt
```

If `cmp` produces no output, the files are identical.

---

## Synchronization Operations

The synchronization engine currently supports two operation types.

### COPY

A `COPY` operation reuses data that already exists inside the destination file.

Conceptually:

```text
COPY(position, size)
```

The operation stores:

- The position inside the destination
- The number of bytes to copy

No additional data needs to be stored because the bytes already exist in the destination.

### INSERT

An `INSERT` operation stores data that is not available in the destination.

Conceptually:

```text
INSERT(data, size)
```

The operation stores:

- The new data
- The size of that data

The data is copied into dynamically allocated memory until synchronization is applied.

---

## Block Processing

The current implementation uses a fixed block size:

```c
#define BLOCK_SIZE 4
```

For example, a source file containing:

```text
ABCDEF
```

is divided into:

```text
ABCD
EF
```

The final block can be smaller than the normal block size.

Each block stores:

- Block index
- Block size
- Checksum

---

## Checksum

The project uses a simple additive checksum.

For a block:

```text
ABC
```

the checksum is conceptually:

```text
'A' + 'B' + 'C'
```

The checksum is useful as a fast way to identify potential matches.

However, it is not guaranteed to uniquely identify a block.

---

## Rolling Checksum

When searching the destination, recalculating the checksum for every possible window would be inefficient.

Instead, the implementation uses a rolling checksum.

Suppose the current window is:

```text
ABCD
```

and the window moves one position:

```text
BCDE
```

The checksum can be updated by:

```text
new_checksum =
    old_checksum
    - outgoing_byte
    + incoming_byte
```

Instead of recalculating the entire window.

This makes scanning the destination more efficient.

---

## Checksum Collision Handling

Two different byte sequences can have the same additive checksum.

For example, different arrangements of the same byte values can produce the same sum.

Therefore:

```text
Checksum Match
      |
      v
Verify Actual Bytes
      |
   +--+--+
   |     |
 Match  Mismatch
   |     |
   v     v
 COPY   Continue Search
```

A checksum match is therefore only considered a potential match.

The implementation performs a byte-by-byte comparison before creating a `COPY` operation.

This prevents checksum collisions from causing incorrect synchronization.

---

## Synchronization Statistics

The program reports:

- Source size
- Destination size
- Operations
- Bytes copied
- Bytes inserted
- Bytes transferred

### Source Size

The total size of the source file.

### Destination Size

The original size of the destination file.

### Operations

The number of `COPY` and `INSERT` operations generated.

### Bytes Copied

The amount of data reused from the existing destination.

### Bytes Inserted

The amount of new data inserted into the generated output.

### Bytes Transferred

Currently represents the amount of data contained in `INSERT` operations.

---

## Temporary File Safety

The synchronization result is not written directly into the destination.

Instead, the program first writes the result to:

```text
sync_output.tmp
```

The process is:

```text
Destination
     |
     v
Generate synchronization operations
     |
     v
Apply operations
     |
     v
sync_output.tmp
     |
     v
rename()
     |
     v
Destination
```

If synchronization fails:

```text
sync_output.tmp
```

is removed.

The original destination is therefore not partially overwritten by synchronization output.

After successful synchronization, `rename()` replaces the destination with the completed output.

---

## File I/O

The project provides a small file I/O abstraction through:

```text
include/file.h
src/file.c
```

It provides functions for:

```c
open_for_read()
open_for_write()
read_file()
write_file()
close_file()
```

The `read_file()` and `write_file()` functions handle partial system calls by continuing until the requested amount of data has been processed or an error occurs.

Interrupted system calls (`EINTR`) are retried.

This makes the higher-level synchronization code simpler and more robust.

---

## Testing

The project contains tests for the synchronization engine and CLI behavior.

Tests cover:

- COPY operations
- INSERT operations
- Shifted data
- Synchronization output
- Synchronization statistics
- Checksum collision handling
- Identical files
- Completely different files
- Empty source
- Empty destination
- Both files empty
- Source larger than destination
- Source smaller than destination
- Missing source
- End-to-end synchronization
- Temporary-file replacement

### Building the Tests

Compile the synchronization test:

```bash
gcc -Wall -Wextra -Wpedantic -Iinclude \
tests/test_sync.c \
src/file.c \
src/block.c \
src/rolling_checksum.c \
src/sync.c \
-o test_sync
```

Run:

```bash
./test_sync
```

Expected output includes:

```text
PASS: sync operation application
PASS: sync operation tests
PASS: synchronization statistics
```

---

## End-to-End Testing

A complete synchronization test can be performed using:

```bash
printf "This is the source file.\nIt contains some data.\n" \
> tests/integration_source.txt
```

Create a different destination:

```bash
printf "Old destination content.\n" \
> tests/integration_destination.txt
```

Run:

```bash
./mini-rsync \
tests/integration_source.txt \
tests/integration_destination.txt
```

Verify:

```bash
cmp \
tests/integration_source.txt \
tests/integration_destination.txt
```

No output from `cmp` means synchronization succeeded.

---

## Edge Cases Tested

The implementation has been tested against several cases.

### Identical Files

The destination already contains the same content as the source.

### Completely Different Files

No reusable blocks are available and `INSERT` operations are generated.

### Source Larger Than Destination

Additional source data must be inserted.

### Source Smaller Than Destination

The generated destination is reduced to match the source.

### Empty Source

The final destination becomes empty.

### Empty Destination

The source data must be inserted.

### Both Files Empty

The synchronization completes without data operations.

### Checksum Collision

A checksum match with different bytes is rejected after byte-level verification.

### Shifted Data

Existing data can be found at different positions in the destination.

---

## Current Limitations

This project intentionally keeps the implementation small and educational.

### 1. Fixed Block Size

The block size is currently hard-coded:

```c
#define BLOCK_SIZE 4
```

A production implementation would likely use configurable block sizes.

### 2. Maximum Number of Operations

The CLI currently uses:

```c
#define MAX_OPERATIONS 100
```

Therefore, a maximum of 100 synchronization operations can currently be generated in one run.

Dynamic operation allocation can be added later.

### 3. Simple Checksum

The current checksum is a simple additive checksum.

Real synchronization systems use stronger checksum strategies.

### 4. Simplified Matching Algorithm

The current implementation searches the destination independently for each source block.

This is simpler to understand but is not as sophisticated as the algorithm used by real rsync implementations.

### 5. No Network Synchronization

The current program works only with local files.

It does not currently support:

- TCP
- UDP
- Remote hosts
- Client/server synchronization
- Network data transfer

Networking is intentionally outside the current scope.

### 6. Destination Must Exist

The current CLI expects the destination file to already exist.

Creating a missing destination automatically is a possible future improvement.

### 7. Temporary Filename

The program currently uses:

```text
sync_output.tmp
```

as its temporary file.

This means concurrent synchronization processes could potentially interfere with each other.

A unique temporary filename would be safer for a production implementation.

### 8. Large File Memory Usage

The current CLI loads both the source and destination files into memory.

This is suitable for a small educational project but is not ideal for very large files.

A future version could process files incrementally.

---

## Design Decisions

Several design decisions were intentionally made to keep the project understandable.

### Fixed Operation Array

The current implementation uses:

```c
#define MAX_OPERATIONS 100
```

rather than dynamically allocating the operation list.

This keeps memory management simpler while the project focuses on the synchronization algorithm.

### Separate Header and Source Files

The project separates declarations and implementations:

```text
include/
src/
```

This keeps the code modular and makes individual components easier to test.

### Temporary Output

Synchronization is performed into a separate temporary file rather than directly modifying the destination.

This reduces the risk of leaving the destination partially synchronized if an error occurs.

### Byte Verification

Checksums are treated as a fast filter rather than proof of equality.

Actual bytes are verified before generating `COPY` operations.

---

## Design Goals

The main goals of this project are to understand and implement:

- File synchronization concepts
- Fixed-size blocks
- Checksums
- Rolling checksums
- Checksum collision handling
- COPY/INSERT operations
- Low-level file I/O
- Memory management in C
- Error handling
- CLI application design
- Automated testing
- Safe temporary-file replacement
- Modular C project organization

---

## Future Improvements

Possible future improvements include:

- Dynamic synchronization operation allocation
- Configurable block size
- Stronger checksum algorithms
- More efficient block matching
- Streaming large files instead of loading them completely
- Unique temporary filenames
- Automatic creation of missing destination files
- More comprehensive automated tests
- Performance benchmarking
- File metadata preservation
- Network-based synchronization
- TCP client/server support
- Parallel processing
- Improved synchronization statistics

---

## Makefile

The project includes a Makefile to simplify compilation.

Build:

```bash
make
```

Clean build artifacts:

```bash
make clean
```

The Makefile allows the project to be built without manually specifying every source file.

---

## Git Workflow

The project was developed using small, focused commits.

Examples include:

```text
feat: add synchronization statistics
feat: improve CLI argument error handling
feat: safely replace destination using temporary file
fix: handle partial file reads and writes
docs: document synchronization algorithm
docs: add project documentation
```

The goal is to keep each commit focused on one logical change.

---

## License

This project is released under the MIT License.

See the `LICENSE` file for the complete license text.

---

## Author

Built as an educational systems-programming project to explore:

- File synchronization algorithms
- Rolling checksums
- Low-level C programming
- File I/O
- Memory management
- Testing
- CLI application development