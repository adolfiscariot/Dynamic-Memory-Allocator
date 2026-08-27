let mod;
let _init, _malloc, _free, _reset;
let _heap_get_header, _heap_get_end, _freelist_get_head;
let _block_get_size, _block_is_free, _block_get_next, _block_get_adjacent;

export async function initBridge() {
    mod = await AllocatorModule();

    _init   = mod.cwrap('allocator_init',   null,     []);
    _malloc = mod.cwrap('allocator_malloc', 'number', ['number']);
    _free   = mod.cwrap('allocator_free',   'number', ['number']);
    _reset  = mod.cwrap('allocator_reset',  null,     []);

    _heap_get_header    = mod.cwrap('heap_get_header',    'number', []);
    _heap_get_end       = mod.cwrap('heap_get_end', 'number', []);
    _freelist_get_head  = mod.cwrap('freelist_get_head',  'number', []);
    _block_get_size     = mod.cwrap('block_get_size',     'number', ['number']);
    _block_is_free      = mod.cwrap('block_is_free',      'number', ['number']);
    _block_get_next     = mod.cwrap('block_get_next',     'number', ['number']);
    _block_get_adjacent = mod.cwrap('block_get_adjacent', 'number', ['number']);

    _init();
}

export function malloc(size) { return _malloc(size); }
export function free(ptr)    { return _free(ptr); }
export function reset()      { return _reset(); }

export function snapshotHeap() {
    const blocks = [];
    const end = _heap_get_end();
    let cur = _heap_get_header();

    while (cur !== 0 && cur < end) {
        const size = _block_get_size(cur);
        const free = _block_is_free(cur);
        blocks.push({ ptr: cur, size, free });
        cur = _block_get_adjacent(cur);
    }

    return blocks;
}

export function snapshotFreeList() {
    const chain = [];
    let cur = _freelist_get_head();

    while (cur !== 0) {
        chain.push({ ptr: cur, size: _block_get_size(cur) });
        cur = _block_get_next(cur);
    }

    return chain;
}
