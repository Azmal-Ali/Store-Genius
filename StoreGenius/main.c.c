/* ============================================================
 * Inventory Management System (C, console-based) -- SINGLE FILE
 * Capstone Project -- Data Structures & Algorithms using C
 *
 * This is a single-file version of the project for easy copy/paste,
 * review, or compilation. It's functionally identical to the modular
 * version (split across src/ and include/).
 *
 * Build:
 *   gcc -Wall -Wextra -std=c11 -o inventory_system inventory_system.c
 * Run:
 *   ./inventory_system
 *
 * DSA concepts demonstrated:
 *   - Structures           : Item
 *   - Dynamic Memory Alloc : malloc/free throughout
 *   - Linked List          : master storage (ItemList / Node)
 *   - Stack                : undo-delete (UndoStack)
 *   - Hashing              : O(1) avg search by id (HashTable)
 *   - Binary Search Tree   : sorted/recursive search (BSTNode)
 *   - Sorting (QuickSort)  : recursive, comparator-based
 *   - Searching            : linear search + binary search
 *   - Recursion            : quicksort, BST insert/search/inorder
 *   - File Handling        : persistent binary storage
 * ============================================================ */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
 * SECTION 1: Item struct
 * ============================================================ */

#define NAME_LEN 50
#define CATEGORY_LEN 30

typedef struct Item {
    int   id;
    char  name[NAME_LEN];
    char  category[CATEGORY_LEN];
    int   quantity;
    float price;
} Item;

void printItemHeader(void) {
    printf("%-6s %-22s %-14s %-10s %-10s\n", "ID", "Name", "Category", "Qty", "Price");
    printf("--------------------------------------------------------------------\n");
}

void printItemRow(const Item *item) {
    printf("%-6d %-22s %-14s %-10d %-10.2f\n",
           item->id, item->name, item->category, item->quantity, item->price);
}

/* ============================================================
 * SECTION 2: Linked List (master in-memory storage)
 * ============================================================ */

#define DATA_FILE "inventory.dat"

typedef struct Node {
    Item data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
    int   count;
} ItemList;

void initList(ItemList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

/* Linear search by id -- O(n). Used by add (duplicate check),
 * update, and delete. */
Node *findNodeById(ItemList *list, int id) {
    Node *cur = list->head;
    while (cur != NULL) {
        if (cur->data.id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

int insertItem(ItemList *list, Item item) {
    if (findNodeById(list, item.id) != NULL) {
        return 0; /* duplicate id not allowed */
    }

    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return 0;
    }
    node->data = item;
    node->next = NULL;

    if (list->head == NULL) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->count++;
    return 1;
}

int updateItem(ItemList *list, int id, Item updated) {
    Node *node = findNodeById(list, id);
    if (node == NULL) return 0;
    updated.id = id; /* id is immutable */
    node->data = updated;
    return 1;
}

int deleteItemById(ItemList *list, int id, Item *removed) {
    Node *cur = list->head;
    Node *prev = NULL;

    while (cur != NULL) {
        if (cur->data.id == id) {
            if (removed != NULL) *removed = cur->data;

            if (prev == NULL) {
                list->head = cur->next;
            } else {
                prev->next = cur->next;
            }
            if (cur == list->tail) {
                list->tail = prev;
            }
            free(cur);
            list->count--;
            return 1;
        }
        prev = cur;
        cur = cur->next;
    }
    return 0;
}

/* Copies the linked list into a freshly malloc'd array so sorting /
 * searching / BST / hash-table code can work on contiguous memory.
 * Caller must free() the returned pointer. */
Item *listToArray(const ItemList *list, int *outCount) {
    *outCount = list->count;
    if (list->count == 0) return NULL;

    Item *arr = (Item *)malloc(sizeof(Item) * (size_t)list->count);
    if (arr == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        *outCount = 0;
        return NULL;
    }

    Node *cur = list->head;
    int i = 0;
    while (cur != NULL) {
        arr[i++] = cur->data;
        cur = cur->next;
    }
    return arr;
}

void freeList(ItemList *list) {
    Node *cur = list->head;
    while (cur != NULL) {
        Node *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

/* ============================================================
 * SECTION 3: Hash Table (O(1) average search by id)
 * ============================================================ */

#define TABLE_SIZE 101   /* prime -> fewer collisions */

typedef struct HashNode {
    Item data;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *buckets[TABLE_SIZE];
} HashTable;

int hashFunction(int id) {
    return id % TABLE_SIZE;
}

void initHashTable(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }
}

void hashInsert(HashTable *table, Item item) {
    int idx = hashFunction(item.id);
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    if (node == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return;
    }
    node->data = item;
    node->next = table->buckets[idx]; /* insert at head of chain, O(1) */
    table->buckets[idx] = node;
}

/* Average O(1): jump straight to the bucket, then walk a short chain. */
Item *hashSearch(HashTable *table, int id) {
    int idx = hashFunction(id);
    HashNode *cur = table->buckets[idx];
    while (cur != NULL) {
        if (cur->data.id == id) return &cur->data;
        cur = cur->next;
    }
    return NULL;
}

void buildHashTable(HashTable *table, const ItemList *list) {
    initHashTable(table);
    Node *cur = list->head;
    while (cur != NULL) {
        hashInsert(table, cur->data);
        cur = cur->next;
    }
}

void freeHashTable(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode *cur = table->buckets[i];
        while (cur != NULL) {
            HashNode *next = cur->next;
            free(cur);
            cur = next;
        }
        table->buckets[i] = NULL;
    }
}

/* ============================================================
 * SECTION 4: Binary Search Tree (recursive, sorted by id)
 * ============================================================ */

typedef struct BSTNode {
    Item data;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

BSTNode *bstInsert(BSTNode *root, Item item) {
    if (root == NULL) {
        BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
        if (node == NULL) {
            fprintf(stderr, "Error: memory allocation failed.\n");
            return NULL;
        }
        node->data = item;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    if (item.id < root->data.id) {
        root->left = bstInsert(root->left, item);
    } else if (item.id > root->data.id) {
        root->right = bstInsert(root->right, item);
    }
    /* equal ids are ignored -- linked list already enforces uniqueness */
    return root;
}

BSTNode *bstSearch(BSTNode *root, int id) {
    if (root == NULL || root->data.id == id) {
        return root;
    }
    if (id < root->data.id) {
        return bstSearch(root->left, id);
    }
    return bstSearch(root->right, id);
}

void bstInorder(const BSTNode *root) {
    if (root == NULL) return;
    bstInorder(root->left);
    printItemRow(&root->data);
    bstInorder(root->right);
}

void bstFree(BSTNode *root) {
    if (root == NULL) return;
    bstFree(root->left);
    bstFree(root->right);
    free(root);
}

/* ============================================================
 * SECTION 5: Sorting (recursive quicksort, comparator-based)
 * ============================================================ */

typedef int (*CompareFunc)(const Item *a, const Item *b);

int compareById(const Item *a, const Item *b)       { return a->id - b->id; }
int compareByPrice(const Item *a, const Item *b)    { return (a->price > b->price) - (a->price < b->price); }
int compareByQuantity(const Item *a, const Item *b) { return a->quantity - b->quantity; }
int compareByName(const Item *a, const Item *b)     { return strcmp(a->name, b->name); }

static void swapItems(Item *a, Item *b) {
    Item tmp = *a;
    *a = *b;
    *b = tmp;
}

static int partition(Item arr[], int low, int high, CompareFunc cmp) {
    Item pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (cmp(&arr[j], &pivot) <= 0) {
            i++;
            swapItems(&arr[i], &arr[j]);
        }
    }
    swapItems(&arr[i + 1], &arr[high]);
    return i + 1;
}

/* Classic recursive quicksort. O(n log n) average, O(n^2) worst case
 * (already-sorted / reverse-sorted input with this pivot choice). */
void quickSort(Item arr[], int low, int high, CompareFunc cmp) {
    if (low < high) {
        int p = partition(arr, low, high, cmp);
        quickSort(arr, low, p - 1, cmp);
        quickSort(arr, p + 1, high, cmp);
    }
}

/* ============================================================
 * SECTION 6: Searching (linear + binary search)
 * ============================================================ */

/* Portable case-insensitive substring search.
 * (Standard C only -- no strcasestr()/<strings.h>, which are
 * POSIX/GNU-only and don't exist on Windows/MinGW compilers.) */
static int containsIgnoreCase(const char *haystack, const char *needle) {
    if (*needle == '\0') return 1; /* empty needle always matches */

    for (; *haystack != '\0'; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h != '\0' && *n != '\0' &&
               tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++;
            n++;
        }
        if (*n == '\0') return 1; /* matched the whole needle */
    }
    return 0;
}

/* Case-insensitive substring search over the unsorted array. O(n). */
int linearSearchByName(const Item arr[], int n, const char *name) {
    for (int i = 0; i < n; i++) {
        if (containsIgnoreCase(arr[i].name, name)) {
            return i;
        }
    }
    return -1;
}

/* Requires arr[] to already be sorted by id. O(log n). */
int binarySearchById(const Item arr[], int n, int id) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid].id == id) return mid;
        if (arr[mid].id < id) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

/* ============================================================
 * SECTION 7: Undo Stack (linked-list backed)
 * ============================================================ */

typedef struct StackNode {
    Item data;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int count;
} UndoStack;

void initStack(UndoStack *s) {
    s->top = NULL;
    s->count = 0;
}

void pushUndo(UndoStack *s, Item item) {
    StackNode *node = (StackNode *)malloc(sizeof(StackNode));
    if (node == NULL) {
        fprintf(stderr, "Error: memory allocation failed.\n");
        return;
    }
    node->data = item;
    node->next = s->top;
    s->top = node;
    s->count++;
}

int popUndo(UndoStack *s, Item *item) {
    if (s->top == NULL) return 0;
    StackNode *node = s->top;
    *item = node->data;
    s->top = node->next;
    free(node);
    s->count--;
    return 1;
}

int isStackEmpty(const UndoStack *s) {
    return s->top == NULL;
}

void freeStack(UndoStack *s) {
    StackNode *cur = s->top;
    while (cur != NULL) {
        StackNode *next = cur->next;
        free(cur);
        cur = next;
    }
    s->top = NULL;
    s->count = 0;
}

/* ============================================================
 * SECTION 8: File Handling (persistent binary storage)
 * ============================================================ */

/* Binary format: [int count][Item][Item]...  Simple and fast; O(n). */
int saveToFile(const ItemList *list, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Error: could not open '%s' for writing.\n", filename);
        return 0;
    }

    fwrite(&list->count, sizeof(int), 1, fp);
    Node *cur = list->head;
    while (cur != NULL) {
        fwrite(&cur->data, sizeof(Item), 1, fp);
        cur = cur->next;
    }
    fclose(fp);
    return 1;
}

/* Returns number of records loaded (0 if the file doesn't exist yet --
 * that's a normal "first run" case, not an error). */
int loadFromFile(ItemList *list, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        return 0; /* no saved data yet */
    }

    int fileCount = 0;
    if (fread(&fileCount, sizeof(int), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    int loaded = 0;
    Item item;
    while (fread(&item, sizeof(Item), 1, fp) == 1) {
        if (insertItem(list, item)) loaded++;
    }
    fclose(fp);
    return loaded;
}

/* ============================================================
 * SECTION 9: Input helpers (error handling / validation)
 * ============================================================ */

#define LOW_STOCK_THRESHOLD 5

static void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

static int readInt(const char *prompt) {
    int value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &value) == 1) {
            clearInputBuffer();
            return value;
        }
        printf("  Invalid input. Please enter a whole number.\n");
        clearInputBuffer();
    }
}

static float readFloat(const char *prompt) {
    float value;
    while (1) {
        printf("%s", prompt);
        if (scanf("%f", &value) == 1) {
            clearInputBuffer();
            return value;
        }
        printf("  Invalid input. Please enter a number.\n");
        clearInputBuffer();
    }
}

static void readLine(const char *prompt, char *buf, int len) {
    printf("%s", prompt);
    if (fgets(buf, len, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = '\0'; /* strip trailing newline */
    } else {
        buf[0] = '\0';
    }
}

/* ============================================================
 * SECTION 10: Menu actions
 * ============================================================ */

static void addItemMenu(ItemList *list) {
    Item item;
    memset(&item, 0, sizeof(Item));

    item.id = readInt("Enter Item ID: ");
    if (findNodeById(list, item.id) != NULL) {
        printf("  Error: an item with ID %d already exists.\n", item.id);
        return;
    }
    readLine("Enter Item Name: ", item.name, NAME_LEN);
    readLine("Enter Category: ", item.category, CATEGORY_LEN);
    item.quantity = readInt("Enter Quantity: ");
    item.price = readFloat("Enter Price: ");

    if (item.quantity < 0 || item.price < 0) {
        printf("  Error: quantity/price cannot be negative.\n");
        return;
    }

    if (insertItem(list, item)) {
        printf("  Item added successfully.\n");
    } else {
        printf("  Failed to add item.\n");
    }
}

static void updateItemMenu(ItemList *list) {
    int id = readInt("Enter Item ID to update: ");
    Node *node = findNodeById(list, id);
    if (node == NULL) {
        printf("  Item with ID %d not found.\n", id);
        return;
    }

    Item updated = node->data;
    printf("  Leave a field as -1 (or blank for name/category) to keep it unchanged.\n");

    char nameBuf[NAME_LEN];
    readLine("New Name (blank = keep): ", nameBuf, NAME_LEN);
    if (strlen(nameBuf) > 0) strncpy(updated.name, nameBuf, NAME_LEN - 1);

    char catBuf[CATEGORY_LEN];
    readLine("New Category (blank = keep): ", catBuf, CATEGORY_LEN);
    if (strlen(catBuf) > 0) strncpy(updated.category, catBuf, CATEGORY_LEN - 1);

    int qty = readInt("New Quantity (-1 = keep): ");
    if (qty >= 0) updated.quantity = qty;

    float price = readFloat("New Price (-1 = keep): ");
    if (price >= 0) updated.price = price;

    updateItem(list, id, updated);
    printf("  Item updated successfully.\n");
}

static void deleteItemMenu(ItemList *list, UndoStack *undo) {
    int id = readInt("Enter Item ID to delete: ");
    Item removed;
    if (deleteItemById(list, id, &removed)) {
        pushUndo(undo, removed);
        printf("  Item %d deleted. (Use Undo to restore it.)\n", id);
    } else {
        printf("  Item with ID %d not found.\n", id);
    }
}

static void undoMenu(ItemList *list, UndoStack *undo) {
    Item item;
    if (!popUndo(undo, &item)) {
        printf("  Nothing to undo.\n");
        return;
    }
    if (insertItem(list, item)) {
        printf("  Restored item %d (%s).\n", item.id, item.name);
    } else {
        printf("  Could not restore item %d -- an item with that ID already exists.\n", item.id);
    }
}

static void searchMenu(ItemList *list) {
    printf("  Search by: 1) ID (Hash Table)  2) ID (BST)  3) Name (Linear Search)\n");
    int choice = readInt("  Choice: ");

    if (choice == 1) {
        HashTable table;
        buildHashTable(&table, list);
        int id = readInt("  Enter ID: ");
        Item *found = hashSearch(&table, id);
        if (found) { printItemHeader(); printItemRow(found); }
        else printf("  Not found.\n");
        freeHashTable(&table);

    } else if (choice == 2) {
        BSTNode *root = NULL;
        Node *cur = list->head;
        while (cur != NULL) { root = bstInsert(root, cur->data); cur = cur->next; }
        int id = readInt("  Enter ID: ");
        BSTNode *found = bstSearch(root, id);
        if (found) { printItemHeader(); printItemRow(&found->data); }
        else printf("  Not found.\n");
        bstFree(root);

    } else if (choice == 3) {
        int n;
        Item *arr = listToArray(list, &n);
        char name[NAME_LEN];
        readLine("  Enter (partial) name: ", name, NAME_LEN);
        int idx = linearSearchByName(arr, n, name);
        if (idx >= 0) { printItemHeader(); printItemRow(&arr[idx]); }
        else printf("  Not found.\n");
        free(arr);

    } else {
        printf("  Invalid choice.\n");
    }
}

static void displayAllMenu(ItemList *list) {
    if (list->count == 0) { printf("  Inventory is empty.\n"); return; }
    printf("  Sorted by ID (in-order BST traversal):\n\n");
    BSTNode *root = NULL;
    Node *cur = list->head;
    while (cur != NULL) { root = bstInsert(root, cur->data); cur = cur->next; }
    printItemHeader();
    bstInorder(root);
    bstFree(root);
}

static void sortMenu(ItemList *list) {
    if (list->count == 0) { printf("  Inventory is empty.\n"); return; }

    printf("  Sort by: 1) Price  2) Quantity  3) Name  4) ID\n");
    int choice = readInt("  Choice: ");
    CompareFunc cmp = compareById;
    if (choice == 1) cmp = compareByPrice;
    else if (choice == 2) cmp = compareByQuantity;
    else if (choice == 3) cmp = compareByName;

    int n;
    Item *arr = listToArray(list, &n);
    quickSort(arr, 0, n - 1, cmp);
    printItemHeader();
    for (int i = 0; i < n; i++) printItemRow(&arr[i]);
    free(arr);
}

static void reportMenu(ItemList *list) {
    if (list->count == 0) { printf("  Inventory is empty.\n"); return; }

    int n;
    Item *arr = listToArray(list, &n);

    double totalValue = 0.0;
    int totalUnits = 0;
    int lowStockCount = 0;

    for (int i = 0; i < n; i++) {
        totalValue += (double)arr[i].price * arr[i].quantity;
        totalUnits += arr[i].quantity;
        if (arr[i].quantity < LOW_STOCK_THRESHOLD) lowStockCount++;
    }

    printf("  ---- Inventory Report ----\n");
    printf("  Total distinct items : %d\n", n);
    printf("  Total units in stock : %d\n", totalUnits);
    printf("  Total inventory value: %.2f\n", totalValue);
    printf("  Low-stock items (< %d units): %d\n", LOW_STOCK_THRESHOLD, lowStockCount);

    if (lowStockCount > 0) {
        printf("\n  Low-stock details:\n");
        printItemHeader();
        for (int i = 0; i < n; i++) {
            if (arr[i].quantity < LOW_STOCK_THRESHOLD) printItemRow(&arr[i]);
        }
    }
    free(arr);
}

static void printMenu(void) {
    printf("\n==================== INVENTORY MANAGEMENT SYSTEM ====================\n");
    printf(" 1. Add Item            2. Update Item         3. Delete Item\n");
    printf(" 4. Search Item         5. Display All Items    6. Sort & Display\n");
    printf(" 7. Undo Last Delete    8. Generate Report       9. Save\n");
    printf(" 0. Save & Exit\n");
    printf("=======================================================================\n");
}

/* ============================================================
 * SECTION 11: main()
 * ============================================================ */

int main(void) {
    ItemList list;
    UndoStack undo;
    initList(&list);
    initStack(&undo);

    int loaded = loadFromFile(&list, DATA_FILE);
    printf("Loaded %d item(s) from '%s'.\n", loaded, DATA_FILE);

    int running = 1;
    while (running) {
        printMenu();
        int choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addItemMenu(&list); break;
            case 2: updateItemMenu(&list); break;
            case 3: deleteItemMenu(&list, &undo); break;
            case 4: searchMenu(&list); break;
            case 5: displayAllMenu(&list); break;
            case 6: sortMenu(&list); break;
            case 7: undoMenu(&list, &undo); break;
            case 8: reportMenu(&list); break;
            case 9:
                if (saveToFile(&list, DATA_FILE)) printf("  Saved.\n");
                break;
            case 0:
                saveToFile(&list, DATA_FILE);
                printf("  Data saved. Goodbye!\n");
                running = 0;
                break;
            default:
                printf("  Invalid choice, please try again.\n");
        }
    }

    freeList(&list);
    freeStack(&undo);
    return 0;
}
