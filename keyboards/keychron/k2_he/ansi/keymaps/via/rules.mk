VIA_ENABLE = yes

# Let host programs light keys over raw HID (see key_signals.h)
KEY_SIGNALS_ENABLE ?= yes
ifeq ($(strip $(KEY_SIGNALS_ENABLE)), yes)
    SRC += key_signals.c
    OPT_DEFS += -DKEY_SIGNALS_ENABLE
endif
