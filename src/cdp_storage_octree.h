/*
 *  Copyright (c) 2024 Victor M. Barrientos (https://github.com/FirmwGuy/CacadeDP)
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 *  this software and associated documentation files (the "Software"), to deal in
 *  the Software without restriction, including without limitation the rights to
 *  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *  of the Software, and to permit persons to whom the Software is furnished to do
 *  so.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *
 */


typedef struct _cdpOctreeList   cdpOctreeList;
typedef struct _cdpOctreeNode   cdpOctreeNode;

struct _cdpOctreeList {
    cdpOctreeList*  next;           // Next child in current sector.
    cdpOctreeNode*  onode;          // Node owning this list.
    //cdpOctreeList*  self;           // Next self in other sectors.
    //
    cdpRecord       record;         // Child record.
};

typedef struct {
    float           center[3];      // Center of the bounding space (XYZ coords).
    float           subwide;        // Half the width/height/depth of the bounding space.
} cdpOctreeBound;

struct _cdpOctreeNode {
    cdpOctreeNode*  children[8];    // Pointers to child nodes.
    cdpOctreeNode*  parent;         // Pointers to child nodes.
    cdpOctreeList*  list;           // List of records in this node.
    cdpOctreeBound  bound;
};

typedef struct {
    cdpStore        store;        // Parent info.
    //
    cdpOctreeNode   root;         // The root node.
} cdpOctree;


#define EPSILON     (1e-10)




/*
    Octree implementation
*/

static inline cdpOctreeNode* octree_node_new(cdpOctreeNode* parent, cdpOctreeBound* bound) {
    assert(bound && bound->subwide > EPSILON);
    CDP_NEW(cdpOctreeNode, onode);
    onode->parent = parent;
    onode->bound  = *bound;
    return onode;
}


void octree_node_del(cdpOctreeNode* node) {
    if (!node) return;

    for (int i = 0; i < 8; i++) {
        octree_node_del(node->children[i]);
    }

    cdpOctreeList* list = node->list;
    while (list) {
        cdpOctreeList* next = list->next;
        cdp_free(list);
        list = next;
    }

    cdp_free(node);
}


static inline cdpOctree* octree_new(cdpOctreeBound* bound) {
    assert(bound && bound->subwide > EPSILON);

    CDP_NEW(cdpOctree, octree);
    octree->root.bound = *bound;

    return octree;
}


static inline void octree_del(cdpOctree* octree){
    if (!octree) return;
    octree_node_del(&octree->root);
    cdp_free(octree);
}



static inline cdpOctreeNode* octree_list_from_record(cdpRecord* record) {
    return cdp_ptr_dif(record, offsetof(cdpOctreeList, record));
}


#define BOUND_CENTER_QUADRANT(bound, onode, opX, opY, opZ)              \
    do {                                                                \
        bound.center[0] = onode->bound.center[0] opX bound.subwide;     \
        bound.center[1] = onode->bound.center[1] opY bound.subwide;     \
        bound.center[2] = onode->bound.center[2] opZ bound.subwide;     \
    } while(0)


static inline cdpRecord* octree_sorted_insert(cdpOctree* octree, cdpRecord* record, cdpCompare compare, void* context) {
    CDP_NEW(cdpOctreeList, list);
    cdp_record_transfer(record, &list->record);

    cdpOctreeNode* onode = &octree->root;
    unsigned n;
    do {
        for (n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                if (0 < compare(list->record, context, &onode->children[n]->bound)) {
                    onode = onode->children[n];
                    break;
                }
            } else {
                cdpOctreeBound bound;
                bound.subwide = onode->subwide / 2.0f;
                assert(bound.subwide > EPSILON);

                switch (n) {
                  case 0:   BOUND_CENTER_QUADRANT(bound, onode, +, +, +);   break;
                  case 1:   BOUND_CENTER_QUADRANT(bound, onode, +, -, +);   break;
                  case 2:   BOUND_CENTER_QUADRANT(bound, onode, -, -, +);   break;
                  case 3:   BOUND_CENTER_QUADRANT(bound, onode, -, +, +);   break;
                  case 4:   BOUND_CENTER_QUADRANT(bound, onode, +, +, -);   break;
                  case 5:   BOUND_CENTER_QUADRANT(bound, onode, +, -, -);   break;
                  case 6:   BOUND_CENTER_QUADRANT(bound, onode, -, -, -);   break;
                  case 7:   BOUND_CENTER_QUADRANT(bound, onode, -, +, -);   break;
                }

                if (0 < compare(list->record, context, &bound)) {
                    onode->children[n] = otree_node_new(onode, &bound);
                    onode = onode->children[n];
                    break;
                }
            }
        }
    } while (n < 8);

    list->onode = onode;
    list->next  = onode->list;
    onode->list = list;

    return &list->record;
}


static inline bool octree_traverse(cdpOctree* octree, cdpTraverse func, void* context, cdpEntry* entry) {
    assert(octree && func);

    cdpOctreeNode* onode = &octree->root;
    unsigned depth = 0;

    entry->parent = octree->store.owner;
    entry->depth  = 0;
    entry->next   = onode;
    do {
        // Process all records in current node
        for (cdpOctreeList* list = onode->list;  list;  list = list->next) {
            entry->record = entry->next;
            entry->next   = &list->record;
            if (!func(entry, context))
                return true;
            entry->position++;
            entry->prev = entry->record;
        }

        // Move to the first child sector
        bool hasChild = false;
        unsigned n;
        for (n = 0;  n < 8;  n++) {
            if (onode->children[n]) {
                onode = onode->children[n];
                depth++;
                entry->depth = depth;
                hasChild = true;
                break;
            }
        }
        if (!hasChild) {
            // Backtrack to the next sibling or parent
            while (onode && onode->parent) {
                for (n = 0;  n < 8;  n++) {
                    if (onode->parent->children[n] == onode) {
                        break;
                    }
                }
                // Find the next sibling
                n++;
                while (n < 8  &&  !onode->parent->children[n]) {
                    n++;
                }
                if (n < 8) {
                    onode = onode->parent->children[n];
                    break;
                }
                // If no more siblings, ascend to parent
                onode = onode->parent;
                depth--;
                entry->depth = depth;
            }
        }
    } while (onode);

    entry->record = entry->next;
    entry->next   = NULL;
    return func(entry, context);
}


static inline cdpRecord* octree_find_by_name(cdpOctree* octree, cdpID id) {
    cdpEntry entry = {0};
    if (!octree_traverse(octree, octree->store.chdCount, (cdpFunc) rb_traverse_func_break_at_name, cdp_v2p(id), &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_find_by_key(cdpOctree* octree, cdpRecord* key, cdpCompare compare, void* context) {
    cdpEntry entry = {0};
    if (!octree_traverse(octree, octree->store.chdCount, (cdpFunc) compare, key, &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_find_by_position(cdpOctree* octree, size_t position) {
    cdpEntry entry = {0};
    if (!octree_traverse(octree, octree->store.chdCount, (void*) rb_traverse_func_break_at_position, cdp_v2p(position), &entry))
        return entry.record;
    return NULL;
}


static inline cdpRecord* octree_first(cdpOctree* octree) {
    return octree_find_by_position(octree, 0);
}


static inline cdpRecord* octree_last(cdpOctree* octree) {
    return octree_find_by_position(octree, octree->store.chdCount - 1);
}

static inline cdpRecord* octree_prev(cdpRecord* record) {
    cdpOctreeList* list     = octree_list_from_record(record);
    cdpOctreeNode* node     = list->onode;
    cdpOctreeList* current  = node->list;
    cdpOctreeList* previous = NULL;

    while (current && current != list) {
        previous = current;
        current = current->next;
    }
    if (previous)
        return &previous->record;

    // Backtrack to find the previous record in the sequence
    while (node->parent) {
        unsigned n = 0;
        for (;  n < 8;  n++) {
            if (node->parent->children[n] == node)
                break;
        }
        while (n > 0) {
            n--;
            cdpOctreeNode* sibling = node->parent->children[n];
            if (sibling) {
                // Find the last record in the deepest subtree of this sibling
                node = sibling;
                for (;;) {
                    bool hasChild = false;
                    for (int i = 7;  i >= 0;  i--) {
                        if (node->children[i]) {
                            node = node->children[i];
                            hasChild = true;
                            break;
                        }
                    }
                    if (!hasChild)
                        break;
                }

                // Find the last record in this node's list
                cdpOctreeList* last = node->list;
                while (last && last->next) {
                    last = last->next;
                }
                if (last)
                    return &last->record;
            }
        }

        node = node->parent;
    }

    return NULL;
}


static inline cdpRecord* octree_next(cdpRecord* record) {
    cdpOctreeList* list = octree_list_from_record(record);
    cdpOctreeNode* node = list->onode;
    cdpOctreeList* next = list->next;
    if (next)
        return &next->record;

    while (node) {
        for (unsigned n = 0;  n < 8;  n++) {
            if (node->children[n]) {
                node = node->children[n];
                while (!node->list) {
                    // Continue descending until we find a node with records
                    bool hasChild = false;
                    for (unsigned i = 0;  i < 8;  i++) {
                        if (node->children[i]) {
                            node = node->children[i];
                            hasChild = true;
                            break;
                        }
                    }
                    if (!hasChild)
                        break;
                }
                return node->list? &node->list->record: NULL;
            }
        }

        // Move to the next sibling or backtrack to parent
        while (node->parent) {
            unsigned n = 0;
            for (;  n < 8;  n++) {
                if (node->parent->children[n] == node)
                    break;
            }

            // Check the next sibling
            n++;
            for (;  n < 8;  n++) {
                if (node->parent->children[n]) {
                    node = node->parent->children[n];
                    // Descend to the first record in the subtree of this sibling
                    while (!node->list) {
                        bool hasChild = false;
                        for (unsigned i = 0; i < 8; i++) {
                            if (node->children[i]) {
                                node = node->children[i];
                                hasChild = true;
                                break;
                            }
                        }
                        if (!hasChild)
                            break;
                    }
                    return node->list? &node->list->record: NULL;
                }
            }

            node = node->parent;
        }

        // If we've exhausted all possibilities, break out of the loop
        break;
    }

    return NULL;
}



static inline void octree_remove_record(cdpOctree* octree, cdpRecord* record) {
    assert(octree && record);

    // Locate the owning list entry and its node
    cdpOctreeList* list = octree_list_from_record(record);
    cdpOctreeNode* node = list->onode;
    assert(node);

    // Remove the record from the node's list
    cdpOctreeList** link = &node->list;
    while (*link && *link != list) {
        link = &(*link)->next;
    }
    if (*link) {
        // Found the entry; remove it from the list
        *link = list->next;
        cdp_free(list);
    }

    // Clean up the node if it becomes empty (and propagate up the tree)
    while (node && !node->list) {
        bool hasChildren = false;
        for (unsigned i = 0; i < 8; i++) {
            if (node->children[i]) {
                hasChildren = true;
                break;
            }
        }

        if (hasChildren) {
            // The node has children, so we stop cleaning up
            break;
        }

        // If no children, free the node and continue with the parent
        cdpOctreeNode* parent = node->parent;
        if (parent) {
            // Find this node in the parent's children array
            for (unsigned i = 0; i < 8; i++) {
                if (parent->children[i] == node) {
                    parent->children[i] = NULL;
                    break;
                }
            }
        }

        // Free the node (skip if it's the root node)
        if (node != &octree->root) {
            cdp_free(node);
        }

        node = parent;
    }
}



static inline void octree_take(cdpOctree* octree, cdpRecord* target) {
    cdpRecord* last = octree_last(octree);
    cdp_record_transfer(last, target);
    octree_remove_record(octree, last);
}


static inline void octree_pop(cdpOctree* octree, cdpRecord* target) {
    cdpRecord* first = octree_first(octree);
    cdp_record_transfer(first, target);
    octree_remove_record(octree, first);
}


#define OCTREE_MIN_DEPTH    128

static inline void octree_del_all_children(cdpOctree* octree) {
    assert(octree);

    cdpOctreeNode* root = &octree->root;
    cdpOctreeNode* stack[OCTREE_MIN_DEPTH]; // Fixed-size stack for iterative traversal
    unsigned top = 0;

    // Push all root children onto the stack
    for (unsigned i = 0; i < 8; i++) {
        if (root->children[i]) {
            stack[top++] = root->children[i];
            root->children[i] = NULL; // Disconnect child from root
        }
    }

    // Iteratively process the stack
    while (top > 0) {
        // Pop a node from the stack
        cdpOctreeNode* node = stack[--top];

        // Free all records in the node's list
        cdpOctreeList* list = node->list;
        while (list) {
            cdpOctreeList* next = list->next;
            cdp_free(list);
            list = next;
        }

        // Push all children onto the stack
        for (unsigned i = 0; i < 8; i++) {
            if (node->children[i]) {
                stack[top++] = node->children[i];
                assert(top < OCTREE_MIN_DEPTH); // Ensure the stack doesn't overflow
            }
        }

        // Free the node
        cdp_free(node);
    }
}

