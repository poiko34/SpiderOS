#include "heap.h"
#include "pmm.h"

#define PAGE_SIZE 4096
#define MIN_SPLIT 32

typedef struct block_hdr {
    uint32_t size;            // размер payload (без заголовка)
    uint8_t  free;
    uint8_t  _pad[3];
    struct block_hdr* prev;
    struct block_hdr* next;
} block_hdr_t;

static uint32_t heap_base;
static uint32_t heap_end_limit;
static uint32_t heap_reserved_end; // сколько реально "подкрепили" страницами

static block_hdr_t* head;

static inline uint32_t align_up(uint32_t x, uint32_t a) {
    return (x + a - 1) & ~(a - 1);
}

static inline uint32_t hdr_size(void) {
    return (uint32_t)sizeof(block_hdr_t);
}

static void heap_reserve_until(uint32_t need_end)
{
    while (need_end > heap_reserved_end) {
        uint32_t frame = pmm_alloc_frame();
        if (frame == 0) {
            for(;;) { __asm__ volatile("hlt"); }
        }
        // identity-map => ничего мапить не надо, просто считаем что память "есть"
        (void)frame;
        heap_reserved_end += PAGE_SIZE;
    }
}

static block_hdr_t* payload_to_hdr(void* p) {
    return (block_hdr_t*)((uint8_t*)p - sizeof(block_hdr_t));
}

static void split_block(block_hdr_t* b, uint32_t need)
{
    // b свободен и b->size >= need
    // если остаток слишком мал — не делим
    uint32_t remain = b->size - need;
    if (remain < hdr_size() + MIN_SPLIT) return;

    uint8_t* b_payload = (uint8_t*)b + hdr_size();
    block_hdr_t* n = (block_hdr_t*)(b_payload + need);

    n->size = remain - hdr_size();
    n->free = 1;
    n->prev = b;
    n->next = b->next;

    if (b->next) b->next->prev = n;
    b->next = n;

    b->size = need;
}

static void coalesce(block_hdr_t* b)
{
    // слить с next
    if (b->next && b->next->free) {
        block_hdr_t* n = b->next;
        b->size = b->size + hdr_size() + n->size;
        b->next = n->next;
        if (b->next) b->next->prev = b;
    }
    // слить с prev
    if (b->prev && b->prev->free) {
        block_hdr_t* p = b->prev;
        p->size = p->size + hdr_size() + b->size;
        p->next = b->next;
        if (p->next) p->next->prev = p;
    }
}

void heap_init(uint32_t heap_start_phys, uint32_t heap_size_bytes)
{
    heap_base = align_up(heap_start_phys, 16);
    heap_end_limit = heap_start_phys + heap_size_bytes;

    heap_reserved_end = align_up(heap_base, PAGE_SIZE);

    // Подкрепим хотя бы 1 страницу, чтобы был первый блок
    heap_reserve_until(heap_base + PAGE_SIZE);

    head = (block_hdr_t*)(uintptr_t)heap_base;
    head->size = (heap_reserved_end - heap_base) - hdr_size();
    head->free = 1;
    head->prev = 0;
    head->next = 0;
}

static block_hdr_t* find_fit(uint32_t need)
{
    for (block_hdr_t* b = head; b; b = b->next) {
        if (b->free && b->size >= need) return b;
    }
    return 0;
}

static block_hdr_t* grow_heap(uint32_t need)
{
    // расширяем reserved_end так, чтобы уместить новый блок
    // добавим минимум одну страницу, но лучше по need
    uint32_t want_end = heap_reserved_end + align_up(need + hdr_size(), PAGE_SIZE);
    if (want_end > heap_end_limit) {
        return 0;
    }

    uint32_t old_end = heap_reserved_end;
    heap_reserve_until(want_end);

    // добавим новый свободный блок в конец списка
    block_hdr_t* last = head;
    while (last->next) last = last->next;

    // если последний блок свободен и прилегает — просто расширим его
    uint32_t last_end = (uint32_t)(uintptr_t)last + hdr_size() + last->size;
    if (last->free && last_end == old_end) {
        last->size += (heap_reserved_end - old_end);
        return last;
    }

    block_hdr_t* nb = (block_hdr_t*)(uintptr_t)old_end;
    nb->size = (heap_reserved_end - old_end) - hdr_size();
    nb->free = 1;
    nb->prev = last;
    nb->next = 0;
    last->next = nb;
    return nb;
}

void* kmalloc_aligned(size_t size, size_t align)
{
    if (size == 0) return 0;
    if (align < 16) align = 16;

    // Мы делаем выравнивание простым способом: берём блок чуть больше и подгоняем.
    // Для раннего ядра ок. Позже можно улучшить.
    uint32_t need = (uint32_t)size;
    // добавим запас под возможный align-паддинг
    uint32_t request = need + (uint32_t)align;

    block_hdr_t* b = find_fit(request);
    if (!b) {
        b = grow_heap(request);
        if (!b) return 0;
    }

    // делим так, чтобы в блоке осталось request
    split_block(b, request);
    b->free = 0;

    uint8_t* raw = (uint8_t*)b + hdr_size();
    uintptr_t aligned = (uintptr_t)align_up((uint32_t)(uintptr_t)raw, (uint32_t)align);

    return (void*)aligned;
}

void* kmalloc(size_t size)
{
    return kmalloc_aligned(size, 16);
}

void* kmalloc_page_aligned(size_t size)
{
    return kmalloc_aligned(size, PAGE_SIZE);
}

void kfree(void* ptr)
{
    if (!ptr) return;

    // ВНИМАНИЕ: так как kmalloc_aligned возвращает aligned-указатель внутри payload,
    // то прямое ptr->hdr не всегда верно.
    // Для простоты/надёжности на этом этапе:
    //  - используем kmalloc() (align=16) для всего, что нужно освобождать,
    //  - page_aligned пока НЕ освобождаем через kfree.
    //
    // Поэтому тут делаем безопасный путь: ожидаем, что ptr указывает на начало payload.
    block_hdr_t* b = payload_to_hdr(ptr);
    b->free = 1;
    coalesce(b);
}
