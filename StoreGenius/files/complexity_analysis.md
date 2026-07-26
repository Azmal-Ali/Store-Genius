# Complexity Analysis

Time and space complexity for the major operations in the Inventory
Management System, as required by the capstone deliverables.

## Time Complexity

| Operation                              | Data Structure | Average Case  | Worst Case  |
|------------------------------------------|-----------------|---------------|-------------|
| Add item                                 | Linked List      | O(1)          | O(1)        |
| Add item (duplicate-id check)            | Linked List      | O(n)          | O(n)        |
| Update item (find by id)                 | Linked List      | O(n)          | O(n)        |
| Delete item (find + unlink)              | Linked List      | O(n)          | O(n)        |
| Search by ID                             | Hash Table       | O(1)          | O(n)*       |
| Search by ID                             | BST              | O(log n)      | O(n)**      |
| Search by name (substring)                | Array (linear)   | O(n)          | O(n)        |
| Search by ID (pre-sorted array)          | Binary Search    | O(log n)      | O(log n)    |
| Sort all records                         | QuickSort        | O(n log n)    | O(n²)***    |
| In-order display (sorted by id)          | BST traversal    | O(n)          | O(n)        |
| Undo last delete (push/pop)              | Stack            | O(1)          | O(1)        |
| Save all records to disk                 | File I/O         | O(n)          | O(n)        |
| Load all records from disk               | File I/O         | O(n)          | O(n)        |

\* Worst case for the hash table occurs when many ids collide into the same
bucket (all keys hashing to one slot), degrading the chain to a linked list.
With `TABLE_SIZE = 101` (prime) and reasonably distributed ids this is rare.

\*\* Worst case for the BST occurs when ids are inserted in already-sorted
order, degenerating the tree into a linked list (no self-balancing is
implemented — see Future Scope for AVL/Red-Black tree as an enhancement).

\*\*\* QuickSort's worst case happens on already-sorted or reverse-sorted
input with the last-element pivot strategy used here.

## Space Complexity

| Structure                      | Space     | Notes |
|--------------------------------|-----------|-------|
| Linked List (master store)      | O(n)      | one node per item |
| Hash Table                     | O(n)      | rebuilt on demand from the linked list |
| BST                             | O(n)      | rebuilt on demand from the linked list |
| Array (for sort/search)         | O(n)      | temporary copy via `listToArray` |
| Undo Stack                      | O(k)      | k = number of pending undoable deletes |
| Recursion stack (QuickSort)     | O(log n) avg, O(n) worst | call depth |
| Recursion stack (BST ops)       | O(log n) avg, O(n) worst | call depth |

## Notes on Design Trade-offs

- The hash table and BST are **rebuilt from the linked list on demand**
  (each time a search/sort/display action runs) rather than kept
  permanently in sync with every add/update/delete. This trades a little
  extra CPU time per operation for much simpler, easier-to-reason-about
  code — a reasonable trade-off at the scale of a console inventory tool.
- Using the linked list as the single source of truth avoids data
  duplication bugs between three structures that would otherwise need to
  be updated in lockstep on every mutation.
- QuickSort's pivot could be improved (median-of-three, or switching to
  IntroSort) to avoid the O(n²) worst case on adversarial input; this is
  noted as a possible future enhancement.
