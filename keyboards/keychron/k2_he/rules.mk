# Joystick and XInput are disabled to free USB endpoints for MIDI and mouse keys
JOYSTICK_ENABLE = no
JOYSTICK_DRIVER = digital
MIDI_ENABLE = yes
XINPUT_ENABLE = no
MOUSEKEY_ENABLE = yes
KEYCHRON_RGB_ENABLE = yes
# Inscrease stack size to avoid crushing of eeprom_update_block()
USE_PROCESS_STACKSIZE = 0x2000
USE_FPU = yes

OPT_DEFS += -DSHARED_EP_ENABLE -DKEYBOARD_SHARED_EP

include keyboards/keychron/common/analog_matrix/analog_matrix.mk
include keyboards/keychron/common/keychron_common.mk
include keyboards/keychron/common/wireless/wireless.mk

VPATH += $(TOP_DIR)/keyboards/keychron

OPT = 2
