#include "list.h"
#include "thread.h"
#include <stdarg.h>

typedef struct {
    DLLNode *front;
    DLLNode *back;
    long size;
    comparison cmp;
} QuarterMergeData;

typedef struct {
    QuarterMergeData *first;
    QuarterMergeData *second;
} HalfMergeData;


void merge(DLLNode **leftStart, DLLNode **leftEnd, DLLNode *rightStart, DLLNode *rightEnd, comparison cmp);
void merge_prev(DLLNode **leftStart, DLLNode **leftEnd, DLLNode *rightStart, DLLNode *rightEnd, comparison cmp);
void mergesort(DLLNode **front, DLLNode **back, comparison cmp, long size, bool handlePrev, int step);
void *start_half_thread(void *arg);
void *start_quarter_thread(void *arg);
long *_split_into_four(DLLNode **q1, DLLNode **q2, DLLNode **q3, DLLNode **q4, long size, comparison cmp);
long *_split_into_two(DLLNode **q1, DLLNode **q2, long size, comparison cmp);
DLLNode *_list_insert_elem(List *l, DLLNode *pos, void *value, bool before);
DLLNode *_list_insert_builtin(List *l, DLLNode *pos, void *arr, int start, int n, bool sorted);
DLLNode *_list_insert_list(List *l, DLLNode *pos, List *other, DLLNode *start, DLLNode *end, bool sorted);
DLLNode *_list_insert_elem_sorted(List *l, void *value);

/* ------------------------------------------------------------------------- */
/*  Copy/remove helper functions    */
/* ------------------------------------------------------------------------- */

#define list_copy(l, n, value) \
    do { \
        if ((l)->helper.copy) { \
            (l)->helper.copy((n)->data, (value)); \
        } else { \
            memcpy((n)->data, (value), (l)->helper.size); \
        } \
    } while(0)

#define list_rm(l, n) \
    do { \
        if ((l)->helper.del) { \
            (l)->helper.del((n)->data); \
        } \
    } while(0)

/* ------------------------------------------------------------------------- */
/*  DLLNode initializer    */
/* ------------------------------------------------------------------------- */

DLLNode *dll_node_new(size_t size) {
    size_t bytes = sizeof(DLLNode) + size;
    DLLNode *node = malloc(bytes);
    if (!node) {
        DS_OOM();
    }
    memset(node, 0, bytes);
    return node;
}

/* ------------------------------------------------------------------------- */
/*  va_args functions to handle initialization and insertion  */
/* ------------------------------------------------------------------------- */

DLLNode *_list_insert_elem(List *l, DLLNode *pos, void *value, bool before) {
    if (!value) {
        return LIST_ERROR;
    } else if (!pos) {
        list_push_back(l, value);
        return l->back;
    }

    DLLNode *prev = pos->prev, *next = pos->next;
    DLLNode *new = dll_node_new(l->helper.size);
    list_copy(l, new, value);

    if (before) {
        new->next = pos;
        pos->prev = new;
        new->prev = prev;
        if (prev) {
            prev->next = new;
        } else {
            l->front = new;
        }
    } else {
        pos->next = new;
        new->prev = pos;
        new->next = next;
        if (next) {
            next->prev = new;
        } else {
            l->back = new;
        }
    }
    l->size++;
    return new;
}

DLLNode *_list_insert_elem_sorted(List *l, void *value) {
    if (!value) {
        return LIST_ERROR;
    }
    DLLNode *curr = l->front;

    if (!curr || l->helper.cmp(value, curr->data) <= 0) {
        list_push_front(l, value);
        return l->front;
    } else {
        int res = 0;
        DLLNode *prev = l->front;
        curr = curr->next;
        while (curr != NULL) {
            res = l->helper.cmp(value, curr->data);
            if ((res == 0) || ((res < 0) && (l->helper.cmp(value, prev->data) > 0))) {
                return _list_insert_elem(l, curr, value, true);
            }
            prev = prev->next;
            curr = curr->next;
        }
        list_push_back(l, value);
        return l->back;
    }
}

DLLNode *_list_insert_builtin(List *l, DLLNode *pos, void *arr, int start, int n, bool sorted) {
    if (!arr || !n) {
        return LIST_ERROR;
    }

    char *ptr = (char *) arr + (start * l->helper.size);
    DLLNode *rv = LIST_END; /* ListEntry where first element from arr was inserted */
    int endIdx = start + n;
    int i = start;

    if (sorted) {
        for (; i < endIdx; ++i) {
            _list_insert_elem_sorted(l, ptr);
            ptr += l->helper.size;
        }
        rv = l->front;
    } else {
        pos = _list_insert_elem(l, pos, ptr, true);
        ptr += l->helper.size;
        rv = pos;

        for (++i; i < endIdx; ++i) {
            pos = _list_insert_elem(l, pos, ptr, false);
            ptr += l->helper.size;
        }
    }
    return rv;
}

DLLNode *_list_insert_list(List *l, DLLNode *pos, List *other, DLLNode *start, DLLNode *end, bool sorted) {
    if (!other || !(other->front)) {
        return LIST_ERROR;
    } else if (!start) {
        start = other->front;
    }

    DLLNode *curr = start;
    DLLNode *rv = NULL;

    if (sorted) {
        while (curr != end) {
            _list_insert_elem_sorted(l, curr);
            curr = curr->next;
        }
        rv = l->front;
    } else {
        pos = _list_insert_elem(l, pos, curr->data, true);
        rv = pos;
        curr = curr->next;

        while (curr != end) {
            pos = _list_insert_elem(l, pos, curr->data, false);
            curr = curr->next;
        }
    }
    return rv;
}

/* ------------------------------------------------------------------------- */
/*  Main list functions    */
/* ------------------------------------------------------------------------- */

List *list_new(const DSHelper *helper, ListInitializer init, ...) {
    if (!helper || helper->size == 0) {
        return NULL;
    }

    List *l = malloc(sizeof(List));
    if (!l) {
        DS_OOM();
    }
    memset(l, 0, sizeof(List));
    l->helper = *(helper);

    if (init == LIST_INIT_EMPTY) { /* nothing more to do in this case */
        return l;
    }

    int n;
    void *other;

    /* parse arguments */
    va_list args;
    va_start(args, init);

    other = va_arg(args, void *);

    if (init == LIST_INIT_BUILTIN) {
        n = va_arg(args, int);
    }

    va_end(args);

    if (init == LIST_INIT_BUILTIN) {
        _list_insert_builtin(l, LIST_END, other, 0, n, false);
    } else {
        _list_insert_list(l, LIST_END, (List *) other, ((List *) other)->front, LIST_END, false);
    }
    return l;
}

void list_free(List *l) {
    list_clear(l);
    free(l);
}

void _list_push_val(List *l, const void *value, bool front) {
    if (!value) {
        return;
    }
    DLLNode *new = dll_node_new(l->helper.size);
    list_copy(l, new, value);

    if (!(l->front)) {
        l->front = new;
        l->back = new;
    } else {
        if (front) {
            new->next = l->front;
            l->front->prev = new;
            l->front = new;
        } else {
            new->prev = l->back;
            l->back->next = new;
            l->back = new;
        }
    }
    l->size++;
}

void _list_pop_val(List *l, bool front) {
    if (!(l->front)) {
        return;
    }
    DLLNode *repl = front ? l->front : l->back;

    if (front) {
        l->front = repl->next;
        if (l->front) {
            l->front->prev = NULL;
        } else {
            l->back = NULL;
        }
    } else {
        l->back = repl->prev;
        if (l->back) {
            l->back->next = NULL;
        } else {
            l->front = NULL;
        }
    }
    list_rm(l, repl);
    free(repl);
    l->size--;
}

DLLNode *list_insert(List *l, DLLNode *pos, bool sorted, ListInsertType type, ...) {    
    void *value;
    int builtin_start;
    int builtin_n;
    void *l_start;
    void *l_end;

    va_list args;
    va_start(args, type);

    value = va_arg(args, void *);

    if (type == LIST_INSERT_BUILTIN) {
        builtin_start = va_arg(args, int);
        builtin_n = va_arg(args, int);
    } else if (type == LIST_INSERT_LIST) {
        l_start = va_arg(args, void *);
        l_end = va_arg(args, void *);
    }

    va_end(args);

    DLLNode *rv = LIST_END;

    switch (type) {
        case LIST_INSERT_SINGLE:
            rv = sorted ? _list_insert_elem_sorted(l, value) : _list_insert_elem(l, pos, (DLLNode *)value, true);
            break;
        case LIST_INSERT_BUILTIN:
            rv = _list_insert_builtin(l, pos, value, builtin_start, builtin_n, sorted);
            break;
        case LIST_INSERT_LIST:
            rv = _list_insert_list(l, pos, (List *) value, (DLLNode *) l_start, (DLLNode *) l_end, sorted);
            break;
    }
    return rv;
}

DLLNode *list_erase(List *l, DLLNode *first, DLLNode *last) {
    if (!first || !l->front || (first == last)) {
        return LIST_ERROR;
    }

    DLLNode *before = first->prev;
    DLLNode *tmp;

    while (first != last) {
        tmp = first->next;
        list_rm(l, first);
        free(first);
        first = tmp;
        l->size--;
    }

    if (before) {
        before->next = last;
    } else {
        l->front = last;
    }

    DLLNode *res;

    if (last) {
        res = last;
        last->prev = before;
    } else {
        res = LIST_END;
        l->back = before;
    }
    return res;
}

/* ------------------------------------------------------------------------- */
/*  List utility functions    */
/* ------------------------------------------------------------------------- */

void list_reverse(List *l) {
    DLLNode *newFront = l->back;
    DLLNode *newBack = l->front;
    DLLNode *prev = NULL;
    DLLNode *curr = l->front;
    DLLNode *next = NULL;

    while (curr) {
        prev = curr->prev;
        next = curr->next;
        curr->next = prev;
        curr->prev = next;
        curr = next;
    }
    l->front = newFront;
    l->back = newBack;
}

void list_sort(List *l) {
    if (l->front == l->back || !l->helper.cmp) {
        return;
    } else if (l->size == 2 && (l->helper.cmp(l->front->data, l->back->data) > 0)) {
        DLLNode *temp = l->back;
        l->front = l->back;
        l->back = temp;
        l->front->prev = l->back->next = NULL;
        l->front->next = l->back;
        l->back->prev = l->front;
        return;
    } else if (l->size >= 1000000) {
        // QuarterMergeData first = {l->front, l->back, 0, l->helper.cmp}, second, third, fourth;
        // second.cmp = third.cmp = fourth.cmp = l->helper.cmp;
        // HalfMergeData left = {&first, &second}, right = {&third, &fourth};
        // long *arr = _split_into_four(&(first.front), &(second.front), &(third.front), &(fourth.front), (long) l->size, l->helper.cmp);
        // first.size = arr[0];
        // second.size = arr[1];
        // third.size = arr[2];
        // fourth.size = arr[3];
        // free(arr);
        // Thread thread_right;
        // thread_create(&thread_right, NULL, start_half_thread, &right);
        // start_half_thread(&left);
        // thread_join(thread_right, NULL);
        // merge_prev(&((left.first)->front), &((left.first)->back), ((right.first)->front), ((right.first)->back), l->helper.cmp);
        // l->front = &(*(left.first)->front);
        // l->back = &(*(left.first)->back);

        QuarterMergeData first = {l->front, l->back, 0, l->helper.cmp}, second;
        second.cmp = l->helper.cmp;
        //HalfMergeData left = {&first, &second}, right = {&third, &fourth};
        long *arr = _split_into_two(&(first.front), &(second.front), (long) l->size, l->helper.cmp);
        first.size = arr[0];
        second.size = arr[1];
        free(arr);
        Thread thread_right;
        thread_create(&thread_right, NULL, start_quarter_thread, &second);
        mergesort(&(first.front), &(first.back), l->helper.cmp, first.size, false, 2);
        //start_half_thread(&left);
        thread_join(thread_right, NULL);
        merge_prev(&((first.front)), &((first.back)), ((second.front)), ((second.back)), l->helper.cmp);
        l->front = first.front;
        l->back = first.back;


        return;
    }
    mergesort(&(l->front), &(l->back), l->helper.cmp, list_size(l), true, 1);
}

typedef enum {
    LIST_RM_OP_UNIQUE,
    LIST_RM_OP_VALUE,
    LIST_RM_OP_COND
} ListRemovalOp;

void _list_removal_ops(List *l, void *val, meetsCondition cond) {
    if (!(l->front)) {
        return;
    }

    ListRemovalOp op = (!val && !cond) ? LIST_RM_OP_UNIQUE : ((!cond) ? LIST_RM_OP_VALUE : LIST_RM_OP_COND);

    DLLNode *curr = (op == LIST_RM_OP_UNIQUE) ? l->front->next : l->front;
    DLLNode *prev = (op == LIST_RM_OP_UNIQUE) ? l->front : NULL;
    DLLNode *next;
    int res;

    while (curr) {
        next = curr->next;

        switch (op) {
        case LIST_RM_OP_UNIQUE:
            res = !(l->helper.cmp(prev->data, curr->data));
            break;
        case LIST_RM_OP_VALUE:
            res = !(l->helper.cmp(val, curr->data));
            break;
        default:
            res = cond(curr->data);
            break;
        }

        if (res) {
            if (prev) {
                prev->next = next;
            } else {
                l->front = next;
            }

            if (next) {
                next->prev = prev;
            } else {
                l->back = prev;
            }

            list_rm(l, curr);
            free(curr);
            l->size--;
            curr = next;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    l->back = prev;
}

DLLNode *list_find(List *l, void *val) {
    DLLNode *curr = l->front;
    while (curr) {
        if (l->helper.cmp(curr->data, val) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

List *list_sublist(List *this, DLLNode *first, DLLNode *last) {
    if (!this->front || !first || (first == last)) {
        return NULL;
    }

    List *sub = list_new(&(this->helper), LIST_INIT_EMPTY);

    while (first != last) {
        list_push_back(sub, first->data);
        first = first->next;
    }
    return sub;
}

void list_merge(List *this, List *other) {
    if (!other || !other->front) { /* nothing to merge */
        return;
    } else if (!this->front) { /* "this" is empty, set it to other and return */
        this->front = other->front;
        this->back = other->back;
        this->size = other->size;
        other->front = other->back = NULL;
        other->size = 0;
        return;
    }

    merge_prev(&(this->front), &(this->back), other->front, other->back, this->helper.cmp);
    this->size += other->size;
    other->front = other->back = NULL;
    other->size = 0;
}

/* ------------------------------------------------------------------------- */
/*  Sorting helper functions    */
/* ------------------------------------------------------------------------- */

void mergesort(DLLNode **front, DLLNode **back, comparison cmp, long size, bool handlePrev, int step) {
    if (*front == NULL || (*front)->next == NULL) {
        return;
    }

    int maxLen = 2;
    while ((maxLen <<= 1) < size);
    maxLen >>= 1;

    DLLNode *leftStart = NULL, *leftEnd = NULL;
    DLLNode *rightStart = NULL, *rightEnd = NULL;
    DLLNode* prev = NULL, *next;
    bool first;
    bool finalMerge = false;
    int c;
  
    for (int i = step; i < size; i <<= 1) {
        finalMerge = (i == maxLen);
        leftStart = *front;
        while (leftStart) {
            first = (leftStart == *front);

            c = i, leftEnd = leftStart;
            while (--c && leftEnd->next) {
                leftEnd = leftEnd->next;
            }
  
            if ((rightStart = leftEnd->next) == NULL) {
                break;
            }
            leftEnd->next = NULL;

            c = i, rightEnd = rightStart;
            while (--c && rightEnd->next) {
                rightEnd = rightEnd->next;
            }
            next = rightEnd->next;
            rightEnd->next = NULL;

            if (finalMerge) {
                if (handlePrev) {
                    merge_prev(&leftStart, &leftEnd, rightStart, rightEnd, cmp);
                } else {
                    merge(&leftStart, &leftEnd, rightStart, rightEnd, cmp);
                }
                //merge_prev(&leftStart, &leftEnd, rightStart, rightEnd, cmp);
                *front = leftStart;
                *back = leftEnd;
                return;
            }
            merge(&leftStart, &leftEnd, rightStart, rightEnd, cmp);
  
            if (first) {
                *front = leftStart;
            } else {
                prev->next = leftStart;
            }
            prev = leftEnd;
            leftStart = next;
        }
        prev->next = leftStart;
    }
}

void merge(DLLNode **leftStart, DLLNode **leftEnd, DLLNode *rightStart, DLLNode *rightEnd, comparison cmp) {
    DLLNode *mergedFront, *curr;
    DLLNode *left = *leftStart, *right = rightStart;

    int res = cmp(left->data, right->data);
    if (res <= 0) {
        mergedFront = left;
        left = left->next;
    } else {
        mergedFront = right;
        right = right->next;
    }
    curr = mergedFront;

    while (left && right) {
        res = cmp(left->data, right->data);
        if (res <= 0) {
            curr->next = left;
            left = left->next;
        } else {
            curr->next = right;
            right = right->next;
        }
        curr = curr->next;
    }

    if (left) { /* still some elements in left sublist */
        curr->next = left;
    } else { /* still elements in right sublist */
        curr->next = right;
        *leftEnd = rightEnd;
    }
    *leftStart = mergedFront;
}

void merge_prev(DLLNode **leftStart, DLLNode **leftEnd, DLLNode *rightStart, DLLNode *rightEnd, comparison cmp) {
    DLLNode *mergedFront, *curr, *prev = NULL;
    DLLNode *left = *leftStart, *right = rightStart;

    int res = cmp(left->data, right->data);
    if (res <= 0) {
        mergedFront = left;
        left = left->next;
    } else {
        mergedFront = right;
        right = right->next;
    }
    curr = mergedFront;

    while (left && right) {
        res = cmp(left->data, right->data);
        if (res <= 0) {
            curr->next = left;
            left = left->next;
        } else {
            curr->next = right;
            right = right->next;
        }
        curr->prev = prev;
        prev = curr;
        curr = curr->next;
    }

    if (left) { /* still some elements in left sublist */
        while (left) {
            curr->next = left;
            left = left->next;
            curr->prev = prev;
            prev = curr;
            curr = curr->next;
        }
    } else { /* still elements in right sublist */
        while (right) {
            curr->next = right;
            right = right->next;
            curr->prev = prev;
            prev = curr;
            curr = curr->next;
        }
        *leftEnd = rightEnd;
    }
    curr->prev = prev;
    *leftStart = mergedFront;
}


long *_split_into_four(DLLNode **q1, DLLNode **q2, DLLNode **q3, DLLNode **q4, long size, comparison cmp) {
    long *arr = malloc(4 * sizeof(long));
    if (!arr) DS_OOM();
    long half = size >> 1;
    long first = half >> 1;
    long second = half - first;
    long third = (size - half) >> 1;
    long fourth = size - half - third;
    arr[0] = first;
    arr[1] = second;
    arr[2] = third;
    arr[3] = fourth;
    long c = 0;
    DLLNode *curr = *q1, *prev = NULL, *next, *temp;

    long lim = first;
    if (first & 1) {
        lim--;
    }

    if (cmp(curr->data, curr->next->data) > 0) {
        next = curr->next->next;
        temp = curr->next;
        *q1 = temp;
        temp->next = curr;
        curr->next = next;
        curr = temp;
    }
    c += 2;
    prev = curr->next;
    curr = curr->next->next;

    while (c < first) {
        for (; c < lim; c += 2) {
            if (cmp(curr->data, curr->next->data) > 0) {
                next = curr->next->next;
                temp = curr->next;
                prev->next = temp;
                temp->next = curr;
                curr->next = next;
                curr = temp;
            }
            prev = curr->next;
            curr = curr->next->next;
        }
        if (lim < first) {
            temp = curr;
            curr = curr->next;
            temp->next = NULL;
            break;
        } else {
            prev->next = NULL;
        }
    }

    c = 0;
    *q2 = curr;
    lim = second;
    if (second & 1) {
        lim--;
    }

    if (cmp(curr->data, curr->next->data) > 0) {
        next = curr->next->next;
        temp = curr->next;
        *q2 = temp;
        temp->next = curr;
        curr->next = next;
        curr = temp;
    }
    c += 2;
    prev = curr->next;
    curr = curr->next->next;

    while (c < second) {
        for (; c < lim; c += 2) {
            if (cmp(curr->data, curr->next->data) > 0) {
                next = curr->next->next;
                temp = curr->next;
                prev->next = temp;
                temp->next = curr;
                curr->next = next;
                curr = temp;
            }
            prev = curr->next;
            curr = curr->next->next;
        }
        if (lim < second) {
            temp = curr;
            curr = curr->next;
            temp->next = NULL;
            break;
        } else {
            prev->next = NULL;
        }
    }

    c = 0;
    *q3 = curr;
    lim = third;
    if (third & 1) {
        lim--;
    }

    if (cmp(curr->data, curr->next->data) > 0) {
        next = curr->next->next;
        temp = curr->next;
        *q3 = temp;
        temp->next = curr;
        curr->next = next;
        curr = temp;
    }
    c += 2;
    prev = curr->next;
    curr = curr->next->next;

    while (c < third) {
        for (; c < lim; c += 2) {
            if (cmp(curr->data, curr->next->data) > 0) {
                next = curr->next->next;
                temp = curr->next;
                prev->next = temp;
                temp->next = curr;
                curr->next = next;
                curr = temp;
            }
            prev = curr->next;
            curr = curr->next->next;
        }
        if (lim < third) {
            temp = curr;
            curr = curr->next;
            temp->next = NULL;
            break;
        } else {
            prev->next = NULL;
        }
    }

    c = 0;
    *q4 = curr;
    lim = fourth;
    if (fourth & 1) {
        lim--;

    }

    if (cmp(curr->data, curr->next->data) > 0) {
        next = curr->next->next;
        temp = curr->next;
        *q4 = temp;
        temp->next = curr;
        curr->next = next;
        curr = temp;
    }
    c += 2;
    prev = curr->next;
    curr = curr->next->next;

    while (c < fourth) {
        for (; c < lim; c += 2) {
            if (cmp(curr->data, curr->next->data) > 0) {
                next = curr->next->next;
                temp = curr->next;
                prev->next = temp;
                temp->next = curr;
                curr->next = next;
                curr = temp;
            }
            prev = curr->next;
            curr = curr->next->next;
        }
        break;
    }
    return arr;
}

long *_split_into_two(DLLNode **q1, DLLNode **q2, long size, comparison cmp) {
    long *arr = malloc(2 * sizeof(long));
    if (!arr) DS_OOM();
    long half1 = size >> 1;
    long half2 = size - half1;
    arr[0] = half1;
    arr[1] = half2;
    long c = 0;
    DLLNode *curr = *q1, *prev = NULL, *next, *temp;

    long lim = half1;
    if (half1 & 1) {
        lim--;
    }

    if (cmp(curr->data, curr->next->data) > 0) {
        next = curr->next->next;
        temp = curr->next;
        *q1 = temp;
        temp->next = curr;
        curr->next = next;
        curr = temp;
    }
    c += 2;
    prev = curr->next;
    curr = curr->next->next;

    while (c < half1) {
        for (; c < lim; c += 2) {
            if (cmp(curr->data, curr->next->data) > 0) {
                next = curr->next->next;
                temp = curr->next;
                prev->next = temp;
                temp->next = curr;
                curr->next = next;
                curr = temp;
            }
            prev = curr->next;
            curr = curr->next->next;
        }
        if (lim < half1) {
            temp = curr;
            curr = curr->next;
            temp->next = NULL;
            break;
        } else {
            prev->next = NULL;
        }
    }

    c = 0;
    *q2 = curr;
    lim = half2;
    if (half2 & 1) {
        lim--;
    }

    if (cmp(curr->data, curr->next->data) > 0) {
        next = curr->next->next;
        temp = curr->next;
        *q2 = temp;
        temp->next = curr;
        curr->next = next;
        curr = temp;
    }
    c += 2;
    prev = curr->next;
    curr = curr->next->next;

    while (c < half2) {
        for (; c < lim; c += 2) {
            if (cmp(curr->data, curr->next->data) > 0) {
                next = curr->next->next;
                temp = curr->next;
                prev->next = temp;
                temp->next = curr;
                curr->next = next;
                curr = temp;
            }
            prev = curr->next;
            curr = curr->next->next;
        }
        break;
    }
    return arr;
}

void *start_half_thread(void *arg) {
    HalfMergeData *half = (HalfMergeData *) arg;
    Thread second;
    thread_create(&second, NULL, start_quarter_thread, (half->second));
    mergesort(&(half->first->front), &(half->first->back), half->first->cmp, half->first->size, false, 2);
    thread_join(second, NULL);
    merge(&(half->first->front), &(half->first->back), (half->second->front), half->second->back, half->first->cmp);
    return NULL;
}

void *start_quarter_thread(void *arg) {
    QuarterMergeData *quarter = (QuarterMergeData *) arg;
    mergesort(&(quarter->front), &(quarter->back), quarter->cmp, quarter->size, false, 2);
    return NULL;
}
