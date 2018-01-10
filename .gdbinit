  define btload
    file build/bbc-microbit-classic-gcc-nosd/source/pyb-radio-module
    target extended-remote /dev/ttyACM0
    monitor swdp_scan
    attach 1
    set mem inaccessible-by-default off
    mon erase_mass
    load
  end

  define btreset
    set mem inaccessible-by-default off
    set *0xe000ed0c = 0x05fa << 16 | (1 << 2)
  end
