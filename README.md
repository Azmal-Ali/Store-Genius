# Inventory Management System (C)

A console-based Inventory Management System built in C for the **Final Capstone
Project — Industry-Oriented Data Structures & Algorithms Using C**. It lets a user
add, update, delete, search, sort, and report on inventory items, with all data
persisted to disk between runs.

## Features

- Add / Update / Delete / Display inventory records
- Search by ID via a **hash table** (O(1) average) or a **BST** (O(log n) average)
- Search by name via **linear search**
- Sort all records by price, quantity, name, or ID via a recursive **quicksort**
- **Undo** the last delete (backed by a stack)
- Inventory **report**: total units, total value, low-stock alert
- Persistent storage via binary **file handling** (data survives program restarts)
- Menu-driven console interface with input validation

## DSA Concepts Implemented

| Concept                          | Where |
|-----------------------------------|-------|
| Structures                        | `include/item.h` |
| Functions / modular design         | every module in `src/` |
| Dynamic Memory Allocation          | `malloc`/`free` in linked list, hash table, BST, stack |
| File Handling                      | `src/filehandling.c` |
| Searching Algorithm (linear)       | `src/searching.c` → `linearSearchByName` |
| Searching Algorithm (binary)       | `src/searching.c` → `binarySearchById` |
| Sorting Algorithm (quicksort)      | `src/sorting.c` |
| Linked List                        | `src/linkedlist.c` (master storage) |
| Stack                              | `src/stack.c` (undo-delete) |
| Hashing                            | `src/hashtable.c` |
| Binary Search Tree                 | `src/bst.c` |
| Recursion                          | quicksort, BST insert/search/inorder |
| Complexity Analysis                | `docs/complexity_analysis.md` |

## Project Structure

```
InventoryManagementSystem/
├── src/                # .c source files
├── include/            # .h headers
├── docs/               # complexity analysis, notes
├── report/             # project report (PDF)
├── presentation/       # presentation slides
├── screenshots/        # output screenshots
├── sample-data/         # inventory.dat is created here at runtime
├── Makefile
├── README.md
└── LICENSE
```

## Build & Run

Requires `gcc` and `make` (any Linux/macOS/WSL shell, or MinGW on Windows).

```bash
make          # compiles to ./inventory_system
make run      # compiles (if needed) and runs it
make clean    # removes build artifacts
```

Data is saved to `sample-data/inventory.dat` automatically on save/exit, and
reloaded the next time the program starts.

## Menu Overview

```
1. Add Item            2. Update Item          3. Delete Item
4. Search Item         5. Display All Items     6. Sort & Display
7. Undo Last Delete    8. Generate Report       9. Save
0. Save & Exit
```

## Sample Data

`sample-data/sample_items.csv` contains example records you can use as a
reference when manually testing the Add Item flow (the program's own storage
format is a binary `.dat` file, not CSV).

## Known Limitations / Future Scope

See `docs/complexity_analysis.md` and the final presentation slide on
**Future Enhancements** — planned additions include a GUI, database backend
(SQLite), barcode scanning, and multi-user role-based access.

## License

This project is released under the MIT License — see `LICENSE`.
