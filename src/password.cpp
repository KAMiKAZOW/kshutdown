// password.cpp - A basic password protection
// Copyright (C) 2011  Konrad Twardowski
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

#include "password.h"

#include "config.h"
#include "infowidget.h"
#include "log.h"
#include "mainwindow.h"
#include "plugins.h"
#include "utils.h"

#include <QCryptographicHash>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>

#ifdef KS_PURE_QT
	#include <QInputDialog>
#endif // KS_PURE_QT

#ifdef KS_KF5
	#include <KPasswordDialog>
#endif // KS_KF5

// PasswordDialog

// public:

PasswordDialog::PasswordDialog(QWidget *parent) :
	UDialog(parent, i18n("Enter New Password"), false) {
	//qDebug() << "PasswordDialog::PasswordDialog()";

// TODO: show warning if Caps Lock is turned on (no reliable solution in Qt)

	m_password = new QLineEdit();
	m_password->setClearButtonEnabled(true);
	m_password->setEchoMode(QLineEdit::Password);
	connect(m_password, &QLineEdit::textChanged, [this]() {
		updateStatus();
	});

	m_confirmPassword = new QLineEdit();
	m_confirmPassword->setClearButtonEnabled(true);
	m_confirmPassword->setEchoMode(QLineEdit::Password);
	connect(m_confirmPassword, &QLineEdit::textChanged, [this]() {
		updateStatus();
	});

	m_status = new InfoWidget(this);

	auto *layout = new QFormLayout();
	// HACK: force "correct" alignment
	layout->setLabelAlignment(Qt::AlignRight);
	layout->addRow(i18n("Password:"), m_password);
	layout->addRow(i18n("Confirm Password:"), m_confirmPassword);

	mainLayout()->addLayout(layout);
	mainLayout()->addWidget(m_status);

	m_password->setFocus();

	updateStatus();
}

PasswordDialog::~PasswordDialog() {
	//qDebug() << "PasswordDialog::~PasswordDialog()";
	m_password->setText("");
	m_confirmPassword->setText("");
}

void PasswordDialog::apply() {
	Log::warning("Password changed/set by user");

	Config *config = Config::user();
	config->beginGroup("Password Protection");

	QString password = m_password->text();
	config->write("Hash", toHash(password));
	clearPassword(password);

	config->endGroup();
	config->sync();
}

bool PasswordDialog::authorize(QWidget *parent, const QString &caption, const QString &userAction) {
	if (!Config::readBool("Password Protection", userAction, false))
		return true;

	Config *config = Config::user();
	config->beginGroup("Password Protection");
	QString hash = config->read("Hash", "").toString();
	config->endGroup();
	
	if (hash.isEmpty())
		return true;

	QString prompt = i18n("Enter password to perform action: %0").arg(caption);

retry:

	#ifdef KS_KF5
	QScopedPointer<KPasswordDialog> dialog(new KPasswordDialog(parent));
// FIXME: dialog->setPixmap(MainWindow::self()->windowIcon().pixmap(48_px));
	dialog->setPrompt(prompt);
	bool ok = dialog->exec() == KPasswordDialog::Accepted;
	QString password = ok ? dialog->password() : QString();

	if (!ok)
		return false;
	#else
	bool ok;
	QString password = QInputDialog::getText(
		parent,
		QApplication::applicationDisplayName(), // title
		prompt,
		QLineEdit::Password,
		"",
		&ok
	);

	if (!ok)
		return false;
	#endif // KS_KF5

	QString enteredHash = toHash(password);
	clearPassword(password);

	if (hash != enteredHash) {
		Log::warning("Invalid password for action: " + userAction);

		UDialog::error(parent, i18n("Invalid password"));

		goto retry; // goto considered useful
	}

	Log::warning("Action successfully authenticated (using password): " + userAction);

	return true;
}

bool PasswordDialog::authorizeSettings(QWidget *parent) {
	return PasswordDialog::authorize(parent, i18n("Preferences"), "action/settings");
}

void PasswordDialog::clearPassword(QString &password) {
	password.fill('\0');
	password.clear();
}

QString PasswordDialog::toHash(const QString &password) {
	if (password.isEmpty())
		return "";

// TODO: consider other algorithms introduced in Qt 5.x <http://doc.qt.io/qt-5/qcryptographichash.html#Algorithm-enum>
	QString saltedString = "kshutdown-" + password;
	QByteArray saltedArray = saltedString.toUtf8();

	QByteArray hash = QCryptographicHash::hash(saltedArray, QCryptographicHash::Sha1);

	clearPassword(saltedString);
	saltedArray.fill('\0');
	saltedArray.clear();

	return QString(hash.toHex());
}

// private:

void PasswordDialog::updateStatus() {
	QString password = m_password->text();
	QString confirmPassword = m_confirmPassword->text();

	int minLength = 12;
	bool ok = password.length() >= minLength;
	if (!ok) {
		m_status->setText(i18n("Password is too short (need %0 characters or more)").arg(minLength), InfoWidget::Type::Error);
	}
	else {
		ok = (password == confirmPassword);
		if (!ok)
			m_status->setText(i18n("Confirmation password is different"), InfoWidget::Type::Error);
		else
			m_status->setText(QString(), InfoWidget::Type::Error);
	}

	acceptButton()->setEnabled(ok);
	resize(sizeHint());

	clearPassword(password);
	clearPassword(confirmPassword);
}

// PasswordPreferences

// public:

PasswordPreferences::PasswordPreferences(QWidget *parent) :
	QWidget(parent),
	m_configKeyRole(Qt::UserRole)
{
	//qDebug() << "PasswordPreferences::PasswordPreferences()";

	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(10_px);
	mainLayout->setSpacing(10_px);

	m_enablePassword = new QCheckBox(i18n("Enable Password Protection"));
	Config *config = Config::user();
	config->beginGroup("Password Protection");

	m_enablePassword->setChecked(!config->read("Hash", "").toString().isEmpty());
	config->endGroup();
	
	connect(
		m_enablePassword, SIGNAL(clicked(bool)),
		SLOT(onEnablePassword(bool))
	);
	mainLayout->addWidget(m_enablePassword);
	
	QLabel *userActionListLabel = new QLabel(i18n("Password Protected Actions:"));
	mainLayout->addWidget(userActionListLabel);
	
	m_userActionList = new QListWidget();
	m_userActionList->setAlternatingRowColors(true);
	
	addItem("action/settings", i18n("Settings (recommended)"), QIcon("configure"));
	
	for (const Action *action : PluginManager::actionList()) {
		addItem(
			"kshutdown/action/" + action->id(),
			action->originalText(),
			action->icon()
		);
	}

	addItem("kshutdown/action/cancel", i18n("Cancel"), QIcon("dialog-cancel"));
	addItem("action/file_quit", i18n("Quit KShutdown"), QIcon("application-exit"));

	userActionListLabel->setBuddy(m_userActionList);
	mainLayout->addWidget(m_userActionList);

	auto *kioskInfo = new InfoWidget(this);
	kioskInfo->setText("<qt>" + i18n("See Also: %0").arg("<a href=\"https://sourceforge.net/p/kshutdown/wiki/Kiosk/\">Kiosk</a>") + "</qt>", InfoWidget::Type::Info);
	mainLayout->addWidget(kioskInfo);
	
	updateWidgets(m_enablePassword->isChecked());
}

void PasswordPreferences::apply() {
	//qDebug() << "PasswordPreferences::apply()";
	
	Config *config = Config::user();
	config->beginGroup("Password Protection");
	if (!m_enablePassword->isChecked()) {
		Log::warning("Password protection disabled by user");

		config->write("Hash", "");
	}

	int count = m_userActionList->count();
	for (int i = 0; i < count; i++) {
		auto *item = static_cast<QListWidgetItem *>(m_userActionList->item(i));
		QString key = item->data(m_configKeyRole).toString();
		config->write(key, item->checkState() == Qt::Checked);
	}

	config->endGroup();
}

// private:

QListWidgetItem *PasswordPreferences::addItem(const QString &key, const QString &text, const QIcon &icon) {
	auto *item = new QListWidgetItem(text, m_userActionList);
	item->setCheckState(
		Config::readBool("Password Protection", key, false)
		? Qt::Checked
		: Qt::Unchecked
	);
	item->setData(m_configKeyRole, key);
	item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
	item->setIcon(icon);
	
	return item;
}

void PasswordPreferences::updateWidgets(const bool passwordEnabled) {
	m_userActionList->setEnabled(passwordEnabled);
}

// private slots:

void PasswordPreferences::onEnablePassword(bool checked) {
	//qDebug() << "PasswordPreferences::onEnablePassword: " << checked;
	
	if (checked) {
// TODO: consider https://api.kde.org/frameworks/kwidgetsaddons/html/classKNewPasswordDialog.html
// or https://api.kde.org/frameworks/kwidgetsaddons/html/classKPasswordLineEdit.html
		QScopedPointer<PasswordDialog> dialog(new PasswordDialog(this));
		if (dialog->exec() == PasswordDialog::Accepted)
			dialog->apply();
		else
			m_enablePassword->setChecked(false);
	}
	updateWidgets(m_enablePassword->isChecked());
}
