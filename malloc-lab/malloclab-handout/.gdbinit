define xx_align
    p/x (($arg0 + (8 - 1)) & ~0x7)
end

define xx_head
    p/x (char*)$arg0 - 4
end

define xx_foot
    set $hdrp = (char*)$arg0 - 4
    set $bp_size = *(unsigned int*)$hdrp &~ 0x7

    p/x (char*)($arg0) + $bp_size - 8
end

define xx_alloc
    set $hdrp = (char*)$arg0 - 4
    p/x *(unsigned int*)$hdrp & 0x1
end

define xx_size
    set $hdrp = (char*)$arg0 - 4
    p/x *(unsigned int*)$hdrp &~ 0x7
end

define xx_next
    set $hdrp = (char*)$arg0 - 4
    set $bp_size = *(unsigned int*)$hdrp &~ 0x7

    p/x (char*)($arg0) + $bp_size
end

define xx_prev
    set $prev_ftrp = (char*)$arg0 - 8
    set $prev_bp_size = *(unsigned int*)$prev_ftrp &~ 0x7

    p/x (char*)($arg0) - $prev_bp_size
end
