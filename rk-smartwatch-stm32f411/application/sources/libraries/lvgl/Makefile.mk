INCLUDE_FLAGS += -I./sources/libraries/lvgl \
		  -I./sources/libraries/lvgl/src \
          -I./sources/libraries/lvgl/src/core \
          -I./sources/libraries/lvgl/src/display \
		  \
          -I./sources/libraries/lvgl/src/draw \
		  -I./sources/libraries/lvgl/src/draw/dma2d \
		  -I./sources/libraries/lvgl/src/draw/nema_gfx \
		  -I./sources/libraries/lvgl/src/draw/nxp \
		  -I./sources/libraries/lvgl/src/draw/opengles \
		  -I./sources/libraries/lvgl/src/draw/renesas/dave2d \
		  -I./sources/libraries/lvgl/src/draw/sdl \
	      -I./sources/libraries/lvgl/src/draw/sw \
	      -I./sources/libraries/lvgl/src/draw/convert \
	      -I./sources/libraries/lvgl/src/draw/sw/arm2d \
	      -I./sources/libraries/lvgl/src/draw/sw/blend \
	      -I./sources/libraries/lvgl/src/draw/vg_lite \
          \
	      -I./sources/libraries/lvgl/src/drivers/display/lcd \
	      -I./sources/libraries/lvgl/src/drivers/display/st7789 \
		  \
          -I./sources/libraries/lvgl/src/font	\
          -I./sources/libraries/lvgl/src/font/fmt_txt	\
          -I./sources/libraries/lvgl/src/indev \
		  \
          -I./sources/libraries/lvgl/src/layouts \
		  -I./sources/libraries/lvgl/src/layouts/flex \
		  -I./sources/libraries/lvgl/src/layouts/grid \
		  \
		  -I./sources/libraries/lvgl/src/libs/bin_decoder \
		  -I./sources/libraries/lvgl/src/libs/qrcode \
		  \
          -I./sources/libraries/lvgl/src/misc \
          -I./sources/libraries/lvgl/src/misc/cache \
          -I./sources/libraries/lvgl/src/misc/cache/class \
          -I./sources/libraries/lvgl/src/misc/cache/instance \
		  \
		  -I./sources/libraries/lvgl/src/osal \
		  -I./sources/libraries/lvgl/src/others/sysmon \
		  \
		  -I./sources/libraries/lvgl/src/stdlib \
		  -I./sources/libraries/lvgl/src/stdlib/builtin \
		  \
          -I./sources/libraries/lvgl/src/themes \
          -I./sources/libraries/lvgl/src/themes/default \
          -I./sources/libraries/lvgl/src/themes/mono \
          -I./sources/libraries/lvgl/src/themes/simple \
		  \
          -I./sources/libraries/lvgl/src/tick \
          -I./sources/libraries/lvgl/src/widgets/animimage \
          -I./sources/libraries/lvgl/src/widgets/arc \
          -I./sources/libraries/lvgl/src/widgets/animimage \
          -I./sources/libraries/lvgl/src/widgets/bar \
          -I./sources/libraries/lvgl/src/widgets/button \
          -I./sources/libraries/lvgl/src/widgets/buttonmatrix \
          -I./sources/libraries/lvgl/src/widgets/calendar \
          -I./sources/libraries/lvgl/src/widgets/canvas \
          -I./sources/libraries/lvgl/src/widgets/chart \
          -I./sources/libraries/lvgl/src/widgets/checkbox \
          -I./sources/libraries/lvgl/src/widgets/dropdown \
          -I./sources/libraries/lvgl/src/widgets/image \
          -I./sources/libraries/lvgl/src/widgets/imagebutton \
          -I./sources/libraries/lvgl/src/widgets/keyboard \
          -I./sources/libraries/lvgl/src/widgets/label \
          -I./sources/libraries/lvgl/src/widgets/led \
          -I./sources/libraries/lvgl/src/widgets/line \
          -I./sources/libraries/lvgl/src/widgets/list \
          -I./sources/libraries/lvgl/src/widgets/lottie \
          -I./sources/libraries/lvgl/src/widgets/menu \
          -I./sources/libraries/lvgl/src/widgets/msgbox \
          -I./sources/libraries/lvgl/src/widgets/objx_templ \
          -I./sources/libraries/lvgl/src/widgets/property \
          -I./sources/libraries/lvgl/src/widgets/roller \
          -I./sources/libraries/lvgl/src/widgets/scale \
          -I./sources/libraries/lvgl/src/widgets/slider \
          -I./sources/libraries/lvgl/src/widgets/span \
          -I./sources/libraries/lvgl/src/widgets/spinbox \
          -I./sources/libraries/lvgl/src/widgets/spinner \
          -I./sources/libraries/lvgl/src/widgets/switch \
          -I./sources/libraries/lvgl/src/widgets/table \
          -I./sources/libraries/lvgl/src/widgets/tabview \
          -I./sources/libraries/lvgl/src/widgets/textarea \
          -I./sources/libraries/lvgl/src/widgets/tileview \
          -I./sources/libraries/lvgl/src/widgets/win \

LVGL_DIRS := \
	./sources/libraries/lvgl/src \
	./sources/libraries/lvgl/src/core \
	./sources/libraries/lvgl/src/display \
	./sources/libraries/lvgl/src/draw \
	./sources/libraries/lvgl/src/draw/sw \
	./sources/libraries/lvgl/src/draw/convert \
	./sources/libraries/lvgl/src/draw/sw/blend \
	./sources/libraries/lvgl/src/drivers/display/lcd \
	./sources/libraries/lvgl/src/drivers/display/st7789 \
	./sources/libraries/lvgl/src/font \
	./sources/libraries/lvgl/src/font/fmt_txt \
	./sources/libraries/lvgl/src/indev \
	./sources/libraries/lvgl/src/layouts \
	./sources/libraries/lvgl/src/layouts/flex \
	./sources/libraries/lvgl/src/layouts/grid \
	./sources/libraries/lvgl/src/libs/bin_decoder \
	./sources/libraries/lvgl/src/libs/qrcode \
	./sources/libraries/lvgl/src/misc \
	./sources/libraries/lvgl/src/misc/cache \
	./sources/libraries/lvgl/src/misc/cache/class \
	./sources/libraries/lvgl/src/misc/cache/instance \
	./sources/libraries/lvgl/src/osal \
	./sources/libraries/lvgl/src/others/sysmon \
	./sources/libraries/lvgl/src/stdlib \
	./sources/libraries/lvgl/src/stdlib/builtin \
	./sources/libraries/lvgl/src/themes \
	./sources/libraries/lvgl/src/themes/default \
	./sources/libraries/lvgl/src/themes/mono \
	./sources/libraries/lvgl/src/themes/simple \
	./sources/libraries/lvgl/src/tick \
	./sources/libraries/lvgl/src/widgets/animimage \
	./sources/libraries/lvgl/src/widgets/arc \
	./sources/libraries/lvgl/src/widgets/bar \
	./sources/libraries/lvgl/src/widgets/button \
	./sources/libraries/lvgl/src/widgets/buttonmatrix \
	./sources/libraries/lvgl/src/widgets/calendar \
	./sources/libraries/lvgl/src/widgets/canvas \
    ./sources/libraries/lvgl/src/widgets/dropdown \
	./sources/libraries/lvgl/src/widgets/image \
	./sources/libraries/lvgl/src/widgets/imagebutton \
	./sources/libraries/lvgl/src/widgets/label \
	./sources/libraries/lvgl/src/widgets/line \
	./sources/libraries/lvgl/src/widgets/property \
	./sources/libraries/lvgl/src/widgets/roller \
	./sources/libraries/lvgl/src/widgets/scale \
	./sources/libraries/lvgl/src/widgets/slider \
	./sources/libraries/lvgl/src/widgets/spinner	\
	./sources/libraries/lvgl/src/widgets/spinbox \
	./sources/libraries/lvgl/src/widgets/switch	\
	./sources/libraries/lvgl/src/widgets/table \
	./sources/libraries/lvgl/src/widgets/tabview \
	./sources/libraries/lvgl/src/widgets/textarea \
	./sources/libraries/lvgl/src/widgets/tileview \

LVGL_SOURCES := $(foreach dir,$(LVGL_DIRS),$(wildcard $(dir)/*.c))

VPATH += $(LVGL_DIRS)

SOURCE_C += $(notdir $(LVGL_SOURCES))
