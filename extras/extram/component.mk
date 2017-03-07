# Component makefile for extras/extram

# expected anyone using extram driver includes it as 'extram/extram.h'
INC_DIRS += $(extram_ROOT)..

# args for passing into compile rule generation
extram_INC_DIR =
extram_SRC_DIR = $(extram_ROOT)

$(eval $(call component_compile_rules,extram))
