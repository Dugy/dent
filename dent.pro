#-------------------------------------------------
#
# Project created by QtCreator 2016-12-06T16:56:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT += printsupport

QMAKE_CXXFLAGS += -O3

TARGET = dent
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
		mainwindow.cpp \
	parser.cpp \
	task.cpp \
	taskmanager.cpp \
	qcustomplot.cpp \
	settings.cpp \
	derivator.cpp \
	log.cpp \
	integrator.cpp \
	formula.cpp \
	functionfitter.cpp \
	fitter.cpp

HEADERS  += mainwindow.h \
	structs.h \
	parser.h \
	task.h \
	taskmanager.h \
	qcustomplot.h \
	settings.h \
	utils.h \
	derivator.h \
	log.h \
	integrator.h \
	formula.h \
	functionfitter.h \
	fitter.h

FORMS    += mainwindow.ui
