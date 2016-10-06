
bin_SCRIPTS = scripts/*

AM_CPPFLAGS = -D_FILE_OFFSET_BITS=64 -I$(top_srcdir)/sonic -I$(top_srcdir)/sonic/common -I$(top_srcdir)/sonic/private -I$(includedir)/sonic

lib_LTLIBRARIES = libsonic_pas.la
libsonic_pas_la_SOURCES=   src/platform-utils/dn_platform_utils_imp.c
libsonic_pas_la_LDFLAGS= -shared -version-info 1:1:0

#copy sonic-pas systemd service file to target
systemdconfdir=/lib/systemd/system
systemdconf_DATA= config/init/sonic-pas.service


#The CLI used to change levels at runtime
bin_PROGRAMS = sonic_pas_service

nobase_include_HEADERS = sonic/private/dn_pas.h sonic/dn_platform_utils.h

sonic_pas_service_SOURCES = src/pas_lib.c src/pald.c src/pas_monitor/pas_monitor.c src/pas/pas_main.c src/fuse/pas_fuse_main.c \
                       src/pas/pas_chassis_handler.c src/pas/pas_entity_handler.c src/pas/pas_psu_handler.c src/pas/pas_fan_tray_handler.c \
                       src/pas/pas_card_handler.c src/pas/pas_fan_handler.c src/pas/pas_led_handler.c src/pas/pas_display_handler.c \
                       src/pas/pas_temp_threshold_handler.c src/pas/pas_pld_handler.c src/pas/pas_port_module_handler.c src/pas/pas_status_handler.c \
                       src/pas/pas_media_handler.c src/pas/pas_media_channel_handler.c src/pas/pas_phy_handler.c \
                       src/pas/pas_temperature_handler.c src/pas/pas_media_config_handler.c src/pas_data_store.cpp \
                       src/pas_event.c src/pas_entity.c src/pas_psu.c src/pas_fan_tray.c src/pas_card.c src/pas_fan.c src/pas_media.c src/pas_media_utils.c \
                       src/pas_utils.c src/pas_config.c src/pas_temp_sensor.c src/pas_led.c src/pas_display.c

sonic_pas_service_SOURCES+= src/fuse/pas_fuse_fan.c src/fuse/pas_fuse_common.c src/fuse/pas_fuse_led.c src/fuse/pas_fuse_thermal_sensor.c \
                        src/fuse/pas_fuse_display_led.c src/fuse/pas_fuse_entity_info.c src/fuse/pas_fuse_media.c \
			src/fuse/pas_fuse_parser.c src/fuse/pas_fuse_diag_mode.c src/remote_poller/pas_remote_poller.c

sonic_pas_service_CXXFLAGS= -std=c++11
sonic_pas_service_LDADD = -lsonic_pas -lfuse -lsonic_common -lsonic_sdi_sys -lsonic_object_library -lrt -lsonic_logging -lpthread
