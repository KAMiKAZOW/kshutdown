// log.cpp - Log
// Copyright (C) 2015  Konrad Twardowski
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <QDateTime>

#include "log.h"

// private:

QFile *Log::logFile = nullptr;
QTextStream *Log::output = nullptr;

// public:

void Log::error(const QString &text) {
	log("ERROR", text);
}

void Log::info(const QString &text) {
	log("INFO", text);
}

void Log::init() {
// TODO: use QMessageLogger #5.x
/*
	logFile = new QFile("log.txt");

	if (logFile->exists()) {
		logFile->copy("log-previous.txt");
	}

	if (logFile->open(QFile::Truncate | QFile::WriteOnly)) {
		output = new QTextStream(logFile);
		output->setCodec("UTF-8");

		info("KShutdown Startup");
	}
*/
}

void Log::shutDown() {
	if (output) {
		info("KShutdown Exit");

		delete output; // flush
		output = nullptr;
	}
}

void Log::warning(const QString &text) {
	log("WARNING", text);
}

// private:

void Log::log(const QString &category, const QString &text) {
	if (output) {
		QDateTime now = QDateTime::currentDateTime();
		*output << (now.toString(Qt::ISODate) + "  " + category + ": " + text + "\n");
		output->flush();
	}
}
