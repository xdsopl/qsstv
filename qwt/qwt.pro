#-------------------------------------------------
#
# Project created by QtCreator 2014-03-11T09:49:32
#
#-------------------------------------------------
QT += core gui concurrent
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qwt
TEMPLATE = lib
CONFIG += staticlib

QMAKE_CXXFLAGS_DEBUG += -O0 -Wno-implicit-fallthrough
QMAKE_CXXFLAGS_RELEASE-= -O2
QMAKE_CXXFLAGS_RELEASE += -O3 -Wno-implicit-fallthrough

SOURCES += \
    qwt_point_3d.cpp \
    qwt_abstract_legend.cpp \
    qwt_pixel_matrix.cpp \
    qwt_plot_xml.cpp \
    qwt_math.cpp \
    qwt_plot_seriesitem.cpp \
    qwt_legend_data.cpp \
    qwt_point_polar.cpp \
    qwt_transform.cpp \
    qwt_plot_magnifier.cpp \
    qwt_plot_dict.cpp \
    qwt_plot_textlabel.cpp \
    qwt_scale_map.cpp \
    qwt_painter_command.cpp \
    qwt_plot_panner.cpp \
    qwt_event_pattern.cpp \
    qwt_point_data.cpp \
    qwt_column_symbol.cpp \
    qwt_text_label.cpp \
    qwt_scale_div.cpp \
    qwt_matrix_raster_data.cpp \
    qwt_interval_symbol.cpp \
    qwt_round_scale_draw.cpp \
    qwt_plot_directpainter.cpp \
    qwt_interval.cpp \
    qwt_widget_overlay.cpp \
    qwt_spline.cpp \
    qwt_text_engine.cpp \
    qwt_series_data.cpp \
    qwt_legend_label.cpp \
    qwt_plot_picker.cpp \
    qwt_abstract_scale_draw.cpp \
    qwt_plot_grid.cpp \
    qwt_color_map.cpp \
    qwt_curve_fitter.cpp \
    qwt_abstract_scale.cpp \
    qwt_plot_scaleitem.cpp \
    qwt_plot_shapeitem.cpp \
    qwt_raster_data.cpp \
    qwt_magnifier.cpp \
    qwt_panner.cpp \
    qwt_clipper.cpp \
    qwt_picker_machine.cpp \
    qwt_null_paintdevice.cpp \
    qwt_plot_marker.cpp \
    qwt_dyngrid_layout.cpp \
    qwt_plot_zoomer.cpp \
    qwt_plot_rescaler.cpp \
    qwt_plot_item.cpp \
    qwt_text.cpp \
    qwt_plot_axis.cpp \
    qwt_abstract_slider.cpp \
    qwt_point_mapper.cpp \
    qwt_plot_legenditem.cpp \
    qwt_legend.cpp \
    qwt_scale_draw.cpp \
    qwt_scale_widget.cpp \
    qwt_slider.cpp \
    qwt_graphic.cpp \
    qwt_plot_rasteritem.cpp \
    qwt_plot_renderer.cpp \
    qwt_scale_engine.cpp \
    qwt_plot_canvas.cpp \
    qwt_wheel.cpp \
    qwt_plot_curve.cpp \
    qwt_plot.cpp \
    qwt_painter.cpp \
    qwt_picker.cpp \
    qwt_plot_layout.cpp \
    qwt_symbol.cpp

HEADERS += \
    qwt.h \
    qwt_compat.h \
    qwt_clipper.h \
    qwt_global.h \
    qwt_plot_magnifier.h \
    qwt_plot_panner.h \
    qwt_plot_dict.h \
    qwt_plot_textlabel.h \
    qwt_text_label.h \
    qwt_round_scale_draw.h \
    qwt_legend_label.h \
    qwt_plot_seriesitem.h \
    qwt_matrix_raster_data.h \
    qwt_abstract_legend.h \
    qwt_legend_data.h \
    qwt_interval_symbol.h \
    qwt_pixel_matrix.h \
    qwt_dyngrid_layout.h \
    qwt_magnifier.h \
    qwt_spline.h \
    qwt_plot_grid.h \
    qwt_plot_scaleitem.h \
    qwt_raster_data.h \
    qwt_point_mapper.h \
    qwt_scale_div.h \
    qwt_plot_picker.h \
    qwt_abstract_scale.h \
    qwt_panner.h \
    qwt_plot_directpainter.h \
    qwt_scale_draw.h \
    qwt_plot_layout.h \
    qwt_plot_shapeitem.h \
    qwt_math.h \
    qwt_null_paintdevice.h \
    qwt_plot_marker.h \
    qwt_legend.h \
    qwt_slider.h \
    qwt_transform.h \
    qwt_scale_widget.h \
    qwt_point_data.h \
    qwt_abstract_scale_draw.h \
    qwt_plot_legenditem.h \
    qwt_curve_fitter.h \
    qwt_scale_map.h \
    qwt_painter_command.h \
    qwt_column_symbol.h \
    qwt_plot_rescaler.h \
    qwt_point_3d.h \
    qwt_widget_overlay.h \
    qwt_plot_zoomer.h \
    qwt_abstract_slider.h \
    qwt_plot_renderer.h \
    qwt_point_polar.h \
    qwt_plot_rasteritem.h \
    qwt_series_store.h \
    qwt_text_engine.h \
    qwt_wheel.h \
    qwt_plot_canvas.h \
    qwt_samples.h \
    qwt_color_map.h \
    qwt_graphic.h \
    qwt_picker_machine.h \
    qwt_event_pattern.h \
    qwt_painter.h \
    qwt_text.h \
    qwt_symbol.h \
    qwt_scale_engine.h \
    qwt_interval.h \
    qwt_plot_item.h \
    qwt_plot.h \
    qwt_series_data.h \
    qwt_picker.h \
    qwt_plot_curve.h
unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
